# uinx & linux 关于线程的源码分析

1. 线程创建入口 pthread_create.c

```cpp
versioned_symbok(libpthread, __pthread_create_2_1, pthread_create, GLIBC_2_1);

int
__pthread_create_2_1(newthread, attr, start_routine, arg)

```

2. 详细的 __pthread_create_2_1 实现细节

2.1 pd

2.2 分配 pd 内存
> 采用的策略：优先选用户已分批内存，否则选用历史缓存(历史缓存策略，采用最近适配原则，但是有上限限定)

```cpp
int err = ALLOCATE_STACK (iattr, &pd);
```

具体实现源码path: glibc/nptl/allocatestack.c

2.2 flag 配置

```cpp
pd->flags = ((iattr->flags & ~(ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET))
	       | (self->flags & (ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET)));
```


2.3 线程创建

```cpp
retval = create_thread (pd, iattr, STACK_VARIABLES_ARGS);
```


3. 线程创建 create_thread

3.1 配置 clone_flags 
> 核心，用于让内核创建与父线程共用一套 mm_struct
```cpp

int clone_flags = (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGNAL
		     | CLONE_SETTLS | CLONE_PARENT_SETTID
		     | CLONE_CHILD_CLEARTID | CLONE_SYSVSEM
		     | 0);

```

3.2 调用 do_clone 

如果失败，回退，清理 线程的栈空间。

成功，配置CPU 亲和性、调用策略/优先级，以及给父线程同步(你已经是一个对线程了状态)

```cpp

static int
do_clone (struct pthread *pd, const struct pthread_attr *attr,
	  int clone_flags, int (*fct) (void *), STACK_VARIABLES_PARMS,
	  int stopped){
// ....

/* Set the affinity mask if necessary.  */
      if (attr->cpuset != NULL)
	{
	  res = INTERNAL_SYSCALL (sched_setaffinity, err, 3, pd->tid,
				  attr->cpusetsize, attr->cpuset);

	  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (res, err), 0))
	    {
	      /* The operation failed.  We have to kill the thread.  First
		 send it the cancellation signal.  */
	      INTERNAL_SYSCALL_DECL (err2);
	    err_out:
	      (void) INTERNAL_SYSCALL (tgkill, err2, 3,
				       THREAD_GETMEM (THREAD_SELF, pid),
				       pd->tid, SIGCANCEL);

	      /* We do not free the stack here because the canceled thread
		 itself will do this.  */

	      return (INTERNAL_SYSCALL_ERROR_P (res, err)
		      ? INTERNAL_SYSCALL_ERRNO (res, err)
		      : 0);
	    }
	}


THREAD_SETMEM (THREAD_SELF, header.multiple_threads, 1);

// ....


}


```
