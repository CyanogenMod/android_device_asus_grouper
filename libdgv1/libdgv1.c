#define LOG_TAG "libdgv1.so"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <utils/Log.h>
#include <string.h>
#include <dlfcn.h>

/*
 * CURIOUS WHAT THE HELL IS GOING ON IN HERE? READ UP...
 *
 *
 * Problems:
 * 1. Nexus 7's GFX libraries were made to work with android L
 * 2. Android M changed a few things around that make it not work
 *   a. dlopen is now posix compliant and thus things like dlopen("egl/abc.so") will no longer look anywhere except root
 * 3. Due to this, we must do something. GFX libs do not call dlopen() directly, they use libnvos.so
 *
 * Curious data snippets
 *  Due to peculiarities of the ELF format, when a binary baz imports function foo() from libbar.so,
 *   nowhere in baz's ELF file does it say that foo() must from from libbar. In fact there are two
 *   separate records. One that says that libbar is "NEED"ed, and another that says that there is an
 *   import of function "foo". What that means is that if the process wer to also load libxyz, which
 *   also exported foo(), there is no way to be sure which foo() would get called. Why do we care?
 *   Well, consider out problems above. We need to provide functions and variables that existing
 *   libraries no longer do. How?
 *
 * A tricky but clever solution: INTERPOSITION library
 * 1. We'll edit the libnvos.so and replace one of its "NEED" record with one referencing a new library
 *    which we'll create. Need a library name? why not "dmitrygr1.so"? We'll also edit the function name
 *    from dlopen to LibLdr() to make sure our function gets called.
 * 2. Make sure that dmitrygr1.so's NEED records include the library whose record we replaced in libnvos.so
 *    library, to make sure that the linker brings it in afterall and all symbols in it are found
 * 3. Implement dmitrygr1.so such that it provides LibLdr() and backs that wil a proper clal to dlopen()
 *    with eht proper psth
 * 
 * Result: GFX libraries works on M, with the help of dmitrygr1.so and a small binary patch to libnvos.so
 */


typedef uintptr_t NvError;
struct NvOsLibraryHandle;


NvError NvOsLibraryLoad(const char *name, struct NvOsLibraryHandle *library);

NvError dmitrygr_libldr(const char *name, struct NvOsLibraryHandle *library)
{
    static const char *prepend = "/system/lib/";
    char *path;
    NvError err;

    err = NvOsLibraryLoad(name, library);
    if (!err)
        return err;

    //now try full path
    //then try in /system/lib
    path = malloc(strlen(name) + strlen(prepend) + 1);
    if (!path)
        return err;
    sprintf(path, "%s%s", prepend, name);
    err = NvOsLibraryLoad(path, library);
    if (!err)
        ALOGI("Just saved you by loading '%s' instead of '%s'", path, name);
    free(path);

    return err;
}


void libEvtLoading(void) __attribute__((constructor));
void libEvtLoading(void)
{
    ALOGI("Loaded to help save your day\n");

}

