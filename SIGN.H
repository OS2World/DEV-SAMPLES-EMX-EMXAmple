// sign.h  (-*- C++ -*-)

enum signs {negative, zero, positive, undefined};

class sign
{
  signs value;
public:
  sign (signs s = undefined) {value = s;}
  sign (int number);
  ~sign () {}
  friend char to_char (sign s1);
  friend sign operator + (sign s1, sign s2);
  friend sign operator - (sign s1);
  friend sign operator - (sign s1, sign s2);
  friend sign operator * (sign s1, sign s2);
  int operator == (sign s2) {return (value == s2.value);}
  int operator != (sign s2) {return (value != s2.value);}
  friend int operator < (sign s1, sign s2);
  friend int operator <= (sign s1, sign s2);
  friend int operator > (sign s1, sign s2);
  friend int operator >= (sign s1, sign s2);
};
