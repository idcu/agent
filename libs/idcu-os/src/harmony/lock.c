#include "idcu/os.h"

idcu_mutex_t* idcu_mutex_create(void) { return NULL; }
void idcu_mutex_destroy(idcu_mutex_t* mutex) { (void)mutex; }
int idcu_mutex_lock(idcu_mutex_t* mutex) { (void)mutex; return -1; }
int idcu_mutex_unlock(idcu_mutex_t* mutex) { (void)mutex; return -1; }
int idcu_mutex_trylock(idcu_mutex_t* mutex) { (void)mutex; return -1; }

idcu_cond_t* idcu_cond_create(void) { return NULL; }
void idcu_cond_destroy(idcu_cond_t* cond) { (void)cond; }
int idcu_cond_wait(idcu_cond_t* cond, idcu_mutex_t* mutex) { (void)cond; (void)mutex; return -1; }
int idcu_cond_timedwait(idcu_cond_t* cond, idcu_mutex_t* mutex, uint32_t timeout_ms) { (void)cond; (void)mutex; (void)timeout_ms; return -1; }
int idcu_cond_signal(idcu_cond_t* cond) { (void)cond; return -1; }
int idcu_cond_broadcast(idcu_cond_t* cond) { (void)cond; return -1; }

idcu_rwlock_t* idcu_rwlock_create(void) { return NULL; }
void idcu_rwlock_destroy(idcu_rwlock_t* rwlock) { (void)rwlock; }
int idcu_rwlock_rdlock(idcu_rwlock_t* rwlock) { (void)rwlock; return -1; }
int idcu_rwlock_wrlock(idcu_rwlock_t* rwlock) { (void)rwlock; return -1; }
int idcu_rwlock_unlock(idcu_rwlock_t* rwlock) { (void)rwlock; return -1; }
