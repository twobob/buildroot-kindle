#include "fakekey/fakekey.h"

int
main(int argc, char **argv)
{
  Display *dpy;
  FakeKey *fk;
  int      i;
  unsigned char str[] = "hello HELLO worldly world", *p = NULL;
  unsigned char str2[] = "\303\270";

  if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
      fprintf(stderr,"Failed to open display\n");
      exit(1);
    }
  
  fk = fakekey_init(dpy);
  
  p = str;

  /*
  for (i=0; i<10; i++)
    {
      fakekey_press(fk, str2, 2, 0);
      fakekey_release(fk);
    }
  */

  while (*p != '\0')
    {
      fakekey_press(fk, p, 1, 0);
      fakekey_release(fk);
      p++;
    }


}
