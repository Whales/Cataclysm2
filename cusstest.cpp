#include "cuss.h"
#include "window.h"

int main(int argc, char** argv)
{
  init_display();
  cuss::interface cint;
  if (argc < 2) {
    debugmsg("Usage: cusstest <file to test>");
  } else if (!cint.load_from_file(argv[1])) {
    return 1;
  } else {
    Window w(0, 0, 80, 24);
    bool quit = false;
    while (!quit) {
      cint.draw(&w);
      long ch = getch();
      cint.handle_keypress(ch);
      if (ch == '|') {
        quit = true;
      }
    }
  }
  endwin();
  return 0;
}
