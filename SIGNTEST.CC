// signtest.cc

#include <iostream.h>
#include "sign.h"

ostream &operator << (ostream &stream, sign s1)
{
  return (stream << to_char (s1));
}


int main (void)
{
  sign s0;
  sign sp = positive;
  sign s1 = 2;
  sign s2 = -3;
  sign s3 = s2;

  cout << "Expected output: ? + - - + 0 1 -\n";
  cout << "Actual output:   ";
  // Due to a GCC bug, we cannot combine the following code
  // into one expression
  cout << s0 << ' ';
  cout << s1 << ' ';
  cout << s2 << ' ';
  cout << s3 << ' ';
  cout << sp << ' ';
  cout << (s1 < s2);
  cout << ' ' << (s1 > s2);
  cout << ' ' << s1 * s2;
  cout << '\n';
  return (0);
}
