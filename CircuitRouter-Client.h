#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include "lib/commandlinereader.h"
#define CLIENT_PIPE "clientpipe.pipe"
#define BUFFERSIZE 120
#define MAXPARAMS 3