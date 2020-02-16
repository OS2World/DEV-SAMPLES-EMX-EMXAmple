// objcsamp.m (emx+gcc)

#include <stdio.h>
#include <objc/Object.h>

/////////////////////////////////////////////////////////////////////////////

@interface Integer: Object
{
  int value;
}

+ new: (int) i;
- print;
- store: (int) i;
- (int) get;
- add: obj;

@end

/////////////////////////////////////////////////////////////////////////////

@implementation Integer

+ new: (int) i
{
  id instance;

  instance  = [super new];
  [instance store:i];
  return (instance);
}

- store: (int) i
{
  value = i;
  return (self);
}

- (int) get
{
  return (value);
}


- add: obj
{
  value += [obj get];
  return (self);
}


- print
{
  printf ("%d", value);
  return (self);
}

@end

/////////////////////////////////////////////////////////////////////////////

int main (void)
{
  id int_1, int_2;

  int_1 = [Integer new:17];
  int_2 = [Integer new:25];
  [int_1 print];
  [int_1 add:int_2];
  printf ("+%d=%d\n", [int_2 get], [int_1 get]);
  return (0);
}
