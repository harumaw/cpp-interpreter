struct  A
{
  int a;
  struct B{
    int v;
    int c;
  } b; // Add member 'b' of type 'B'
};

A c;
A::B b;

int main() {
    c.b.v = 4;
    return;
}
