#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifndef PUBLIC
#define PUBLIC __attribute__((visibility("default")))
#endif // !PUBLIC

#ifndef PRIVATE
#define PRIVATE __attribute__((visibility("hidden")))
#endif // !PRIVATE

typedef int __pid_t;
typedef int __ssize_t;
typedef unsigned int __id_t;
typedef unsigned int __useconds_t;

#ifndef __pid_t_defined
typedef __pid_t pid_t;
#define __pid_t_defined
#endif

#ifndef __id_t_defined
typedef __id_t id_t;
#define __id_t_defined
#endif

#ifndef __useconds_t_defined
typedef __useconds_t useconds_t;
#define __useconds_t_defined
#endif

#ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
#define __ssize_t_defined
#endif

#endif
