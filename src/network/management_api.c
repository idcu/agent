#include "network/management_api.h"
#include "module/module_registry.h"
#include "module/module_version.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

static const char* g_index_html = 
"<!DOCTYPE html>\n"
"<html lang=\"zh-CN\">\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"    <title>IDCU Agent 管理控制台</title>\n"
"    <style>\n"
"        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
"        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f0f2f5; color: #333; }\n"
"        .header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
"        .header h1 { font-size: 28px; margin-bottom: 5px; }\n"
"        .header p { opacity: 0.9; font-size: 14px; }\n"
"        .container { max-width: 1200px; margin: 30px auto; padding: 0 20px; }\n"
"        .status-card { background: white; border-radius: 12px; padding: 20px; margin-bottom: 20px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); }\n"
"        .status-card h2 { font-size: 18px; margin-bottom: 15px; color: #667eea; display: flex; align-items: center; gap: 10px; }\n"
"        .health-indicator { display: inline-block; width: 12px; height: 12px; border-radius: 50%; animation: pulse 2s infinite; }\n"
"        .health-indicator.healthy { background: #10b981; }\n"
"        .health-indicator.unhealthy { background: #ef4444; }\n"
"        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }\n"
"        .module-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 20px; }\n"
"        .module-card { background: white; border-radius: 12px; padding: 20px; box-shadow: 0 2px 8px rgba(0,0,0,0.08); transition: transform 0.2s, box-shadow 0.2s; }\n"
"        .module-card:hover { transform: translateY(-3px); box-shadow: 0 8px 20px rgba(0,0,0,0.12); }\n"
"        .module-header { display: flex; justify-content: space-between; align-items: start; margin-bottom: 15px; }\n"
"        .module-name { font-size: 18px; font-weight: 600; color: #1f2937; }\n"
"        .module-version { font-size: 12px; color: #6b7280; background: #f3f4f6; padding: 3px 8px; border-radius: 4px; }\n"
"        .module-id { font-size: 13px; color: #9ca3af; margin-bottom: 10px; }\n"
"        .module-state { display: inline-block; padding: 6px 14px; border-radius: 20px; font-size: 12px; font-weight: 500; }\n"
"        .state-running { background: #d1fae5; color: #065f46; }\n"
"        .state-stopped { background: #f3f4f6; color: #4b5563; }\n"
"        .state-error { background: #fee2e2; color: #991b1b; }\n"
"        .state-uninitialized { background: #fef3c7; color: #92400e; }\n"
"        .state-initialized { background: #dbeafe; color: #1e40af; }\n"
"        .module-actions { margin-top: 15px; display: flex; gap: 10px; }\n"
"        .btn { padding: 8px 16px; border: none; border-radius: 6px; font-size: 13px; font-weight: 500; cursor: pointer; transition: all 0.2s; }\n"
"        .btn-primary { background: #667eea; color: white; }\n"
"        .btn-primary:hover { background: #5a67d8; }\n"
"        .btn-danger { background: #ef4444; color: white; }\n"
"        .btn-danger:hover { background: #dc2626; }\n"
"        .btn:disabled { opacity: 0.5; cursor: not-allowed; }\n"
"        .refresh-btn { background: white; color: #667eea; border: 2px solid #667eea; }\n"
"        .refresh-btn:hover { background: #667eea; color: white; }\n"
"        .toolbar { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }\n"
"        .last-update { font-size: 13px; color: #6b7280; }\n"
"        .loading { text-align: center; padding: 40px; color: #6b7280; }\n"
"        .error { background: #fee2e2; color: #991b1b; padding: 15px; border-radius: 8px; margin-bottom: 20px; }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class=\"header\">\n"
"        <h1>🖥️ IDCU Agent 管理控制台</h1>\n"
"        <p>可视化管理和监控您的节点与模块</p>\n"
"    </div>\n"
"    <div class=\"container\">\n"
"        <div class=\"status-card\">\n"
"            <h2><span class=\"health-indicator\" id=\"healthIndicator\"></span> 系统状态</h2>\n"
"            <div id=\"healthStatus\">检查中...</div>\n"
"        </div>\n"
"        <div class=\"status-card\">\n"
"            <div class=\"toolbar\">\n"
"                <h2>📦 模块列表</h2>\n"
"                <div style=\"display: flex; gap: 10px; align-items: center;\">\n"
"                    <span class=\"last-update\" id=\"lastUpdate\"></span>\n"
"                    <button class=\"btn refresh-btn\" onclick=\"refreshModules()\">🔄 刷新</button>\n"
"                </div>\n"
"            </div>\n"
"            <div id=\"moduleContainer\" class=\"loading\">加载中...</div>\n"
"        </div>\n"
"    </div>\n"
"    <script>\n"
"        const API_BASE = '';\n"
"        let modules = [];\n"
"        async function checkHealth() {\n"
"            try {\n"
"                const response = await fetch(API_BASE + '/health');\n"
"                const data = await response.json();\n"
"                const indicator = document.getElementById('healthIndicator');\n"
"                const status = document.getElementById('healthStatus');\n"
"                if (data.status === 'healthy') {\n"
"                    indicator.className = 'health-indicator healthy';\n"
"                    status.textContent = '✓ 系统运行正常';\n"
"                } else {\n"
"                    indicator.className = 'health-indicator unhealthy';\n"
"                    status.textContent = '✗ 系统状态异常';\n"
"                }\n"
"            } catch (error) {\n"
"                const indicator = document.getElementById('healthIndicator');\n"
"                const status = document.getElementById('healthStatus');\n"
"                indicator.className = 'health-indicator unhealthy';\n"
"                status.textContent = '✗ 无法连接到服务器';\n"
"            }\n"
"        }\n"
"        async function fetchModules() {\n"
"            const container = document.getElementById('moduleContainer');\n"
"            try {\n"
"                const response = await fetch(API_BASE + '/api/v1/modules');\n"
"                modules = await response.json();\n"
"                renderModules();\n"
"                updateLastUpdate();\n"
"            } catch (error) {\n"
"                container.innerHTML = '<div class=\"error\">无法加载模块列表: ' + error.message + '</div>';\n"
"            }\n"
"        }\n"
"        function renderModules() {\n"
"            const container = document.getElementById('moduleContainer');\n"
"            if (modules.length === 0) {\n"
"                container.innerHTML = '<div class=\"loading\">暂无模块</div>';\n"
"                return;\n"
"            }\n"
"            container.innerHTML = '<div class=\"module-grid\">' + modules.map(m => `\n"
"                <div class=\"module-card\">\n"
"                    <div class=\"module-header\">\n"
"                        <div>\n"
"                            <div class=\"module-name\">${escapeHtml(m.name)}</div>\n"
"                            <div class=\"module-id\">ID: ${m.id}</div>\n"
"                        </div>\n"
"                        <span class=\"module-version\">v${escapeHtml(m.version)}</span>\n"
"                    </div>\n"
"                    <span class=\"module-state state-${m.state}\">${getStateLabel(m.state)}</span>\n"
"                    <div class=\"module-actions\">\n"
"                        <button class=\"btn btn-primary\" onclick=\"startModule('${escapeHtml(m.name)}')\" \n"
"                            ${m.state === 'running' ? 'disabled' : ''}>▶ 启动</button>\n"
"                        <button class=\"btn btn-danger\" onclick=\"stopModule('${escapeHtml(m.name)}')\" \n"
"                            ${m.state !== 'running' ? 'disabled' : ''}>⏹ 停止</button>\n"
"                    </div>\n"
"                </div>\n"
"            `).join('') + '</div>';\n"
"        }\n"
"        function getStateLabel(state) {\n"
"            const labels = {\n"
"                'uninitialized': '未初始化',\n"
"                'initialized': '已初始化',\n"
"                'running': '运行中',\n"
"                'stopped': '已停止',\n"
"                'error': '错误'\n"
"            };\n"
"            return labels[state] || state;\n"
"        }\n"
"        function escapeHtml(text) {\n"
"            const div = document.createElement('div');\n"
"            div.textContent = text;\n"
"            return div.innerHTML;\n"
"        }\n"
"        function updateLastUpdate() {\n"
"            const now = new Date();\n"
"            document.getElementById('lastUpdate').textContent = '最后更新: ' + now.toLocaleTimeString();\n"
"        }\n"
"        async function startModule(name) {\n"
"            try {\n"
"                const response = await fetch(API_BASE + '/api/v1/modules/' + encodeURIComponent(name) + '/start', { method: 'POST' });\n"
"                const data = await response.json();\n"
"                if (data.success) {\n"
"                    await fetchModules();\n"
"                } else {\n"
"                    alert('启动失败: ' + (data.error || '未知错误'));\n"
"                }\n"
"            } catch (error) {\n"
"                alert('请求失败: ' + error.message);\n"
"            }\n"
"        }\n"
"        async function stopModule(name) {\n"
"            try {\n"
"                const response = await fetch(API_BASE + '/api/v1/modules/' + encodeURIComponent(name) + '/stop', { method: 'POST' });\n"
"                const data = await response.json();\n"
"                if (data.success) {\n"
"                    await fetchModules();\n"
"                } else {\n"
"                    alert('停止失败: ' + (data.error || '未知错误'));\n"
"                }\n"
"            } catch (error) {\n"
"                alert('请求失败: ' + error.message);\n"
"            }\n"
"        }\n"
"        function refreshModules() {\n"
"            fetchModules();\n"
"        }\n"
"        checkHealth();\n"
"        fetchModules();\n"
"        setInterval(checkHealth, 30000);\n"
"        setInterval(fetchModules, 10000);\n"
"    </script>\n"
"</body>\n"
"</html>";


static const char* module_state_to_string(idcu_ModuleState state) {
    switch (state) {
        case IDCU_MOD_STATE_UNINIT: return "uninitialized";
        case IDCU_MOD_STATE_INITED: return "initialized";
        case IDCU_MOD_STATE_RUNNING: return "running";
        case IDCU_MOD_STATE_STOPPED: return "stopped";
        case IDCU_MOD_STATE_ERROR: return "error";
        default: return "unknown";
    }
}

static int handle_get_modules(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    char json[IDCU_HTTP_MAX_BODY];
    int offset = 0;
    
    offset += snprintf(json + offset, sizeof(json) - offset, "[");
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (kernel && kernel->registry) {
        int count = idcu_module_registry_get_count(kernel->registry);
        for (int i = 0; i < count; i++) {
            const idcu_RegisteredModule* mod = idcu_module_registry_get_at(kernel->registry, i);
            if (mod) {
                char ver_str[IDCU_VERSION_STR_MAX];
                idcu_version_format(&mod->iface->version, ver_str, sizeof(ver_str));
                
                if (i > 0) {
                    offset += snprintf(json + offset, sizeof(json) - offset, ",");
                }
                
                offset += snprintf(json + offset, sizeof(json) - offset,
                    "{\"id\":%u,\"name\":\"%s\",\"version\":\"%s\",\"state\":\"%s\"}",
                    mod->module_id, mod->iface->name, ver_str,
                    module_state_to_string(mod->state));
            }
        }
    }
    
    offset += snprintf(json + offset, sizeof(json) - offset, "]");
    
    idcu_http_response_set_json_body(response, json);
    return IDCU_ERR_OK;
}

static int handle_get_module(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (!kernel || !kernel->registry) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Internal server error\"}");
        return IDCU_ERR_OK;
    }
    
    const char* path = request->path;
    const char* module_name = strrchr(path, '/');
    if (!module_name) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_json_body(response, "{\"error\":\"Invalid path\"}");
        return IDCU_ERR_OK;
    }
    module_name++;
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(kernel->registry, module_name);
    if (!mod) {
        idcu_http_response_set_status(response, 404);
        idcu_http_response_set_json_body(response, "{\"error\":\"Module not found\"}");
        return IDCU_ERR_OK;
    }
    
    char json[IDCU_HTTP_MAX_BODY];
    char ver_str[IDCU_VERSION_STR_MAX];
    idcu_version_format(&mod->iface->version, ver_str, sizeof(ver_str));
    
    snprintf(json, sizeof(json),
        "{\"id\":%u,\"name\":\"%s\",\"version\":\"%s\",\"state\":\"%s\"}",
        mod->module_id, mod->iface->name, ver_str,
        module_state_to_string(mod->state));
    
    idcu_http_response_set_json_body(response, json);
    return IDCU_ERR_OK;
}

static int handle_start_module(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (!kernel || !kernel->registry) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Internal server error\"}");
        return IDCU_ERR_OK;
    }
    
    const char* path = request->path;
    const char* module_name = strrchr(path, '/');
    if (!module_name) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_json_body(response, "{\"error\":\"Invalid path\"}");
        return IDCU_ERR_OK;
    }
    module_name++;
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(kernel->registry, module_name);
    if (!mod) {
        idcu_http_response_set_status(response, 404);
        idcu_http_response_set_json_body(response, "{\"error\":\"Module not found\"}");
        return IDCU_ERR_OK;
    }
    
    int ret = idcu_module_registry_init_module(kernel->registry, mod->module_id);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Failed to initialize module\"}");
        return IDCU_ERR_OK;
    }
    
    ret = idcu_module_registry_run_module(kernel->registry, mod->module_id);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Failed to start module\"}");
        return IDCU_ERR_OK;
    }
    
    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_json_body(response, "{\"success\":true,\"message\":\"Module started\"}");
    return IDCU_ERR_OK;
}

static int handle_stop_module(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    
    idcu_MicroKernel* kernel = (idcu_MicroKernel*)user_data;
    if (!kernel || !kernel->registry) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Internal server error\"}");
        return IDCU_ERR_OK;
    }
    
    const char* path = request->path;
    const char* module_name = strrchr(path, '/');
    if (!module_name) {
        idcu_http_response_set_status(response, 400);
        idcu_http_response_set_json_body(response, "{\"error\":\"Invalid path\"}");
        return IDCU_ERR_OK;
    }
    module_name++;
    
    const idcu_RegisteredModule* mod = idcu_module_registry_find_by_name(kernel->registry, module_name);
    if (!mod) {
        idcu_http_response_set_status(response, 404);
        idcu_http_response_set_json_body(response, "{\"error\":\"Module not found\"}");
        return IDCU_ERR_OK;
    }
    
    int ret = idcu_module_registry_stop_module(kernel->registry, mod->module_id);
    if (ret != IDCU_ERR_OK) {
        idcu_http_response_set_status(response, 500);
        idcu_http_response_set_json_body(response, "{\"error\":\"Failed to stop module\"}");
        return IDCU_ERR_OK;
    }
    
    idcu_http_response_set_status(response, 200);
    idcu_http_response_set_json_body(response, "{\"success\":true,\"message\":\"Module stopped\"}");
    return IDCU_ERR_OK;
}

static int handle_get_index(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    idcu_http_response_add_header(response, "Content-Type", "text/html; charset=utf-8");
    idcu_http_response_set_body(response, g_index_html, strlen(g_index_html));
    return IDCU_ERR_OK;
}

static int handle_get_health(idcu_HttpRequest* request, idcu_HttpResponse* response, void* user_data) {
    (void)request;
    (void)user_data;
    
    idcu_http_response_set_json_body(response, "{\"status\":\"healthy\"}");
    return IDCU_ERR_OK;
}

int idcu_management_api_register_routes(idcu_HttpServer* server) {
    if (!server) return IDCU_ERR_INVALID_PARAM;
    
    int ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/",
                                          handle_get_index, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/index.html",
                                          handle_get_index, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/health",
                                          handle_get_health, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/api/v1/modules",
                                          handle_get_modules, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_GET, "/api/v1/modules/*",
                                          handle_get_module, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_POST, "/api/v1/modules/*/start",
                                          handle_start_module, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    ret = idcu_http_server_register_route(server, IDCU_HTTP_METHOD_POST, "/api/v1/modules/*/stop",
                                          handle_stop_module, server->kernel);
    if (ret != IDCU_ERR_OK) return ret;
    
    IDCU_LOG_INFO("Management API routes registered successfully");
    return IDCU_ERR_OK;
}
