#include <QTest>

#include "ihka46_ident.h"
#include "kombi46_ident.h"
#include "dme_ms420_ident.h"
#include "zke5_ident.h"

int main(int argc, char** argv)
{
   int status = 0;

   {
      IHKA46_Ident tc;
      status |= QTest::qExec(&tc, argc, argv);
   }

   {
      KOMBI46_Ident tc;
      status |= QTest::qExec(&tc, argc, argv);
   }

   {
      DME_MS420_Ident tc;
      status |= QTest::qExec(&tc, argc, argv);
   }

   {
       ZKE5_Ident tc;
       status |= QTest::qExec(&tc, argc, argv);
   }
   return status;
}
