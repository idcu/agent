# 任务 3.21: idcu-permission - 权限管理库

&gt; **文档版本**: v2.0  
&gt; **最后更新**: 2026-04-08  
&gt; **责任人**: IDCU Team  
&gt; **任务状态**: ⏳ 待开始

---

## 1. 任务边界

### 1.1 核心目标
创建完整的权限管理库，支持角色管理、权限管理、用户-角色关联、角色-权限关联、权限检查、继承权限、权限缓存、权限审计和持久化，满足权限检查 QPS ≥ 10,000 次/秒，权限缓存命中率 ≥ 90%。

### 1.2 不做什么
- 不实现基于策略的访问控制（PBAC）
- 不实现动态权限更新的实时推送
- 不实现权限报表和可视化
- 不实现权限的版本控制

### 1.3 输入
- 配置参数：缓存启用标志、审计启用标志
- 权限数据：角色定义、权限定义、主体定义、关联关系
- 权限检查请求：主体 ID、资源、动作、上下文

### 1.4 输出
- 角色/权限/主体 ID：创建成功返回唯一 ID
- 权限检查结果：IDCU_ERR_OK 表示允许，其他错误码表示拒绝
- 审计日志：权限检查记录
- 持久化数据：二进制文件或 JSON 格式导出

### 1.5 前置依赖
- idcu-common 基础库（提供向量、哈希表、锁等）
- idcu-storage 库（用于持久化）
- idcu-json 库（用于 JSON 导入导出）
- idcu-log 库（用于日志记录）
- phase2 已完成

---

## 2. 技术实现方案

### 2.1 核心选型
- **数据存储**: 内存中使用 Vector + HashMap，持久化使用 idcu-storage
- **权限缓存**: HashMap 缓存主体-权限映射，LRU 策略（可选）
- **角色继承**: DAG 结构，避免循环依赖
- **审计日志**: Vector 存储，支持导出和清理
- **线程安全**: 互斥锁（idcu_Mutex）保护所有操作

### 2.2 核心逻辑
```
1. 初始化：配置缓存和审计选项，初始化数据结构和锁
2. 角色管理：
   a. 添加/删除角色
   b. 查询角色（按 ID 或名称）
   c. 为角色分配/撤销权限
   d. 设置角色继承关系
3. 权限管理：
   a. 添加/删除权限
   b. 查询权限（按 ID 或名称）
   c. 设置权限描述和条件
4. 主体管理：
   a. 添加/删除主体（用户/服务等）
   b. 查询主体（按 ID 或名称）
   c. 为主体分配/撤销角色
   d. 为主体分配直接权限
5. 权限检查：
   a. 查询缓存，命中则直接返回
   b. 未命中则计算主体的所有权限（包括继承）
   c. 匹配资源和动作，应用条件判断
   d. 更新缓存并返回结果
6. 缓存管理：清除全部缓存、使主体/角色缓存失效
7. 审计管理：启用/禁用审计、获取审计日志、清除审计日志
8. 持久化：保存到文件、从文件加载、JSON 导入导出
9. 销毁：释放所有资源、销毁锁
```

### 2.3 数据结构/接口
```c
// 核心类型定义
typedef uint64_t idcu_RoleId;
typedef uint64_t idcu_PermissionId;
typedef uint64_t idcu_SubjectId;

// 权限效果枚举
typedef enum {
    IDCU_PERMISSION_EFFECT_DENY = 0,
    IDCU_PERMISSION_EFFECT_ALLOW
} idcu_PermissionEffect;

// 权限结构体
typedef struct {
    idcu_PermissionId id;
    char name[128];
    char resource[256];
    char action[64];
    idcu_PermissionEffect effect;
    char description[512];
    char conditions[1024];
} idcu_Permission;

// 角色结构体
typedef struct {
    idcu_RoleId id;
    char name[128];
    char description[512];
    idcu_Vector permissions;
    idcu_Vector parent_roles;
    int is_system;
} idcu_Role;

// 主体结构体
typedef struct {
    idcu_SubjectId id;
    char name[128];
    char type[64];
    idcu_Vector roles;
    idcu_Vector direct_permissions;
} idcu_Subject;

// 权限管理器配置
typedef struct {
    int enable_cache;
    int enable_audit;
} idcu_PermissionManagerConfig;

// 权限管理器
typedef struct {
    idcu_Vector roles;
    idcu_HashMap roles_by_id;
    idcu_HashMap roles_by_name;
    idcu_Vector permissions;
    idcu_HashMap permissions_by_id;
    idcu_HashMap permissions_by_name;
    idcu_Vector subjects;
    idcu_HashMap subjects_by_id;
    idcu_HashMap subjects_by_name;
    idcu_HashMap permission_cache;
    idcu_Mutex lock;
    int audit_enabled;
    idcu_Vector audit_log;
    int initialized;
} idcu_PermissionManager;

// 核心 API
int  idcu_permission_manager_config_init(idcu_PermissionManagerConfig* config);
int  idcu_permission_manager_init(idcu_PermissionManager* manager, const idcu_PermissionManagerConfig* config);
void idcu_permission_manager_destroy(idcu_PermissionManager* manager);

// 角色管理 API
idcu_RoleId idcu_permission_manager_add_role(idcu_PermissionManager* manager, const char* name, const char* description);
int  idcu_permission_manager_remove_role(idcu_PermissionManager* manager, idcu_RoleId id);
idcu_Role* idcu_permission_manager_get_role(idcu_PermissionManager* manager, idcu_RoleId id);
idcu_Role* idcu_permission_manager_get_role_by_name(idcu_PermissionManager* manager, const char* name);
int  idcu_permission_manager_add_role_permission(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_PermissionId permission_id);
int  idcu_permission_manager_remove_role_permission(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_PermissionId permission_id);
int  idcu_permission_manager_add_role_parent(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_RoleId parent_id);
int  idcu_permission_manager_remove_role_parent(idcu_PermissionManager* manager, idcu_RoleId role_id, idcu_RoleId parent_id);
int  idcu_permission_manager_set_system_role(idcu_PermissionManager* manager, idcu_RoleId id, int is_system);

// 权限管理 API
idcu_PermissionId idcu_permission_manager_add_permission(idcu_PermissionManager* manager, const char* name, const char* resource,
                                                         const char* action, idcu_PermissionEffect effect);
int  idcu_permission_manager_remove_permission(idcu_PermissionManager* manager, idcu_PermissionId id);
idcu_Permission* idcu_permission_manager_get_permission(idcu_PermissionManager* manager, idcu_PermissionId id);
idcu_Permission* idcu_permission_manager_get_permission_by_name(idcu_PermissionManager* manager, const char* name);
int  idcu_permission_manager_set_permission_description(idcu_PermissionManager* manager, idcu_PermissionId id,