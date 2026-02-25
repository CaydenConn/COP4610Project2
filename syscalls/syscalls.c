#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/errno.h>

// Function pointers to the handlers, if not loaded then NULL
int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;

// Kernel modules access and assign own handler functions
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

//Syscall 548; Calls module handler if available else return -ENOSYS
SYSCALL_DEFINE0(start_elevator) {
    if (STUB_start_elevator)
        return STUB_start_elevator();
    return -ENOSYS;
}

//Syscall 549; Issues start floor, dest floor, and type else returns _ENOSYS
SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type) {
    if (STUB_issue_request)
        return STUB_issue_request(start_floor, destination_floor, type);
    return -ENOSYS;
}

//Syscall 550; Stops elevator else returns -ENOSYS
SYSCALL_DEFINE0(stop_elevator) {
    if (STUB_stop_elevator)
        return STUB_stop_elevator();
    return -ENOSYS;
}
