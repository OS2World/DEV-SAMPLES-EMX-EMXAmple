// sign.cc

#include "sign.h"

// sign.dll does not need C run-time functions and does not have
// static constructors and destructors.  Therefore no _DLL_InitTerm()
// function is required.

sign operator + (sign s1, sign s2)
{
  static const signs add_tab[4][4] =
    {
      {negative,  negative,  undefined, undefined},
      {negative,  zero,      positive,  undefined},
      {undefined, positive,  positive,  undefined},
      {undefined, undefined, undefined, undefined}
    };

  return (sign (add_tab[s1.value][s2.value]));
}


sign operator - (sign s1)
{
  static const signs neg_tab[4] = {positive, zero, negative, undefined};

  return (sign (neg_tab[s1.value]));
}


sign operator * (sign s1, sign s2)
{
  static const signs mul_tab[4][4] =
    {
      {positive,  zero, negative,  undefined},
      {zero,      zero, zero,      zero},
      {negative,  zero, positive,  undefined},
      {undefined, zero, undefined, undefined}
    };

  return (sign (mul_tab[s1.value][s2.value]));
}


sign operator - (sign s1, sign s2)
{
  return (s1 + (-s2));
}


sign::sign (int number)
{
  if (number > 0)
    value = positive;
  else if (number < 0)
    value = negative;
  else
    value = zero;
}

int operator < (sign s1, sign s2)
{
  return (s1.value == undefined || s2.value == undefined
          ? 0 : s1.value < s2.value);
}


int operator <= (sign s1, sign s2)
{
  return (s2.value == positive ? 1
          : (s1.value == undefined || s2.value == undefined
             ? 0 : s1.value <= s2.value));
}


int operator > (sign s1, sign s2)
{
  return (s1.value == undefined || s2.value == undefined
          ? 0 : s1.value > s2.value);
}


int operator >= (sign s1, sign s2)
{
  return (s1.value == positive ? 1
          : (s1.value == undefined || s2.value == undefined
             ? 0 : s1.value >= s2.value));
}


char to_char (sign s1)
{
  switch (s1.value)
    {
    case positive:
      return ('+');
    case zero:
      return ('0');
    case negative:
      return ('-');
    default:
      return ('?');
    }
}
