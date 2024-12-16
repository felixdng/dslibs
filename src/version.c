
#ifdef VERSION_NUM
#define _VERSION    VERSION_NUM
#else
#define _VERSION    "0.1.0"
#endif
static const char * const soft_version = \
"version_module:V"_VERSION"("__DATE__" "__TIME__")""general";
const char * const get_soft_version(void)
{
    return soft_version;
}

