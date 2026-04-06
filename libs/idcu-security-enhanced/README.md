# idcu-security-enhanced

IDCU 项目的增强安全组件库，提供加密、哈希、随机数生成等功能。

## 特性

- 安全随机数生成
- 密码哈希与验证
- 对称加密（AES-256-CBC、ChaCha20-Poly1305）
- 哈希函数（SHA-256、SHA-512）
- HMAC 消息认证码
- Base64 和 Hex 编码/解码
- 跨平台支持（Windows、Linux、macOS）

## 快速开始

### 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### 安装

```bash
cmake --install .
```

### 在其他项目中使用

```cmake
find_package(idcu-security-enhanced REQUIRED)
add_executable(myapp main.c)
target_link_libraries(myapp PRIVATE idcu::security-enhanced)
```

## API 文档

### 系统初始化和关闭

```c
#include <idcu/security_enhanced/security_enhanced.h>

int idcu_security_init(void);
void idcu_security_shutdown(void);
```

### 随机数生成

```c
int idcu_security_generate_key(uint8_t* key, size_t key_size);
int idcu_security_generate_iv(uint8_t* iv, size_t iv_size);
int idcu_security_generate_salt(uint8_t* salt, size_t salt_size);
int idcu_security_random_bytes(uint8_t* buffer, size_t size);
int idcu_security_random_int(uint64_t* result, uint64_t min, uint64_t max);
```

### 哈希和 HMAC

```c
int idcu_security_hash(const void* data, size_t data_size, uint8_t* hash, size_t* hash_size);
int idcu_security_hmac(const void* data, size_t data_size, const uint8_t* key, size_t key_size,
                        uint8_t* hmac, size_t* hmac_size);
```

### 加密和解密

```c
int idcu_security_encrypt(idcu_SecurityAlgorithm alg,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* iv, size_t iv_size,
                          const void* plaintext, size_t plaintext_size,
                          void* ciphertext, size_t* ciphertext_size);
int idcu_security_decrypt(idcu_SecurityAlgorithm alg,
                          const uint8_t* key, size_t key_size,
                          const uint8_t* iv, size_t iv_size,
                          const void* ciphertext, size_t ciphertext_size,
                          void* plaintext, size_t* plaintext_size);
```

### 密码哈希

```c
int idcu_security_hash_password(const char* password, const uint8_t* salt, size_t salt_size,
                                 uint8_t* hash, size_t* hash_size);
int idcu_security_verify_password(const char* password, const uint8_t* salt, size_t salt_size,
                                    const uint8_t* hash, size_t hash_size);
```

### 编码和解码

```c
int idcu_security_base64_encode(const void* data, size_t data_size, char* output, size_t* output_size);
int idcu_security_base64_decode(const char* input, size_t input_size, void* output, size_t* output_size);
int idcu_security_hex_encode(const void* data, size_t data_size, char* output, size_t* output_size);
int idcu_security_hex_decode(const char* input, size_t input_size, void* output, size_t* output_size);
```

## 测试

```bash
cd build
ctest
```

## 许可证

详见项目根目录的 LICENSE 文件。
