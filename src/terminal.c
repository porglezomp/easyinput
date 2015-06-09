// Code based on example by Nominal Animal in a thread on "Termios examples"

#include <sys/time.h>  // select
#include <sys/types.h> // select
#include <errno.h>     // errno, ENOTTY, ENOTSUP
#include <termios.h>
#include <unistd.h>    // select, ...
#include <signal.h>
#include <stdio.h>

struct termios term_original;
static int term = -1;

static void terminal_done(void) {
  if (term != -1) tcsetattr(term, TCSANOW, &term_original);
  
  // Empty stdout
  fd_set rfds;
  char buf;
  FD_ZERO(&rfds);
  FD_SET(STDIN_FILENO, &rfds);
  struct timeval timeout;
  timeout.tv_sec  = 0, timeout.tv_usec = 100;
  while (select(1, &rfds, NULL, NULL, &timeout)) {
    timeout.tv_sec  = 0, timeout.tv_usec = 100;
    read(STDIN_FILENO, &buf, 1);
  }
}

// Default signal handler: restore terminal and exit
static void terminal_signal(int signum) {
  terminal_done();
  // Common idiom, signal + 128
  // _exit() is acync safe but exit() isn't
  _exit(signum + 128);
}

int setup_terminal(void) {
  // Already set up?
  if (term != -1) return errno = 0;
  
  // Find the terminal file descriptor
  if (isatty(STDIN_FILENO)) {
    term = STDIN_FILENO;
  } else if (isatty(STDOUT_FILENO)) {
    term = STDOUT_FILENO;
  } else if (isatty(STDOUT_FILENO)) {
    term = STDERR_FILENO;
  } else {
    return errno = ENOTTY;
  }
  
  // Store the original settings
  tcgetattr(term, &term_original);

  // Disable buffering
  if (isatty(STDOUT_FILENO)) setvbuf(stdout, NULL, _IONBF, 0);
  if (isatty(STDERR_FILENO)) setvbuf(stderr, NULL, _IONBF, 0);
  if (isatty(STDIN_FILENO)) setvbuf(stdin, NULL, _IONBF, 0);

  // Ensure that the terminal doesn't get stuck in a nonstandard mode
  if (atexit(terminal_done)) return errno = ENOTSUP;

  // Set up the default signal handler
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_handler = terminal_signal;
  act.sa_flags = 0;
  if (sigaction(SIGHUP,  &act, NULL) ||
      sigaction(SIGINT,  &act, NULL) ||
      sigaction(SIGQUIT, &act, NULL) ||
      sigaction(SIGTERM, &act, NULL) ||
#ifdef SIGXCPU
      sigaction(SIGXCPU, &act, NULL) ||
#endif
#ifdef SIGXFSZ
      sigaction(SIGXFSZ, &act, NULL) ||
#endif
#ifdef SIGIO
      sigaction(SIGIO,   &act, NULL) ||
#endif
      sigaction(SIGPIPE, &act, NULL) ||
      sigaction(SIGALRM, &act, NULL)) {
    return errno = ENOTSUP;
  }

  // Set up the terminal settings
  struct termios t = term_original;
  // Let BREAK cause a SIGINT
  t.c_iflag &= ~IGNBRK;
  t.c_iflag |=  BRKINT;

  // Let INTR/QUIT/SUSP/DSUSP generate signals
  t.c_lflag |=  ISIG;

  // Enable noncanonical mode
  // Disables line buffering, etc
  t.c_lflag &= ~ICANON;

  // Disable echoing
  t.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

  // Disable additional input processing
  t.c_lflag &= ~IEXTEN;

  // Update the settings
  tcsetattr(term, TCSANOW, &t);

  return errno = 0;
}

