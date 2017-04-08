#include <stdio.h>
#include <string.h>
 
struct test {
   char  a[6];
   char  b[6];
};
 
int main( ) {

   struct test t;      
   strcpy( t.a, "HELLO");
   strcpy( t.b, "WORLD"); 
 
   printf( "%s ", t.a);
   printf( "%s\n", t.b);
   return 0;
}
