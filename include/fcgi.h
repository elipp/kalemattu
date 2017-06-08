#ifndef FCGI_H
#define FCGI_H

#include <pthread.h>
#include <fcgiapp.h>

#include "types.h"
int start_fcgi_thread(struct kstate_t *state, pthread_t *t);

#endif
