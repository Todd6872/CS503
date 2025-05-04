#include <stdio.h>
#include <string.h>
//void print_dragon();
// EXTRA CREDIT - print the drexel dragon from the readme.md
void print_dragon(){
  // TODO implement 
   //stores compressed dragon image
   char* dragon = " 72@01%04N00 69%06N00 68%06N00 65%01 01%07 11@01N00 64%10 08%07N00"
   " 39%07 02%04@01 09%12@01 04%06 02@01%04N00 34%22 06%28N00 32%26 03%12 01%15N00 31%29 01%19 05%03N00"
   " 29%28@01 01@01%18 08%02N00 28%33 01%22N00 28%58N00 28%50@01%06@01N00 06%08@01 11%16 08%26 06%02N00"
   " 04%13 09%02@01%12 11%11 01%12 06@01%01N00 02%10 03%03 08%14 12%24N00"
   " 01%09 07%01 09%13 13%12@01%11N00%09@01 16%01 01%13 12@01%25N00%08@01 17%02@01%12 12@01%28N00"
   "%07@01 19%15 11%33N00%10 18%15 10%35 06%04N00%09@01 19@01%14 09%12@01 01%04 01%17 03%08N00"
   "%10 18%17 08%13 06%18 01%09N00%09@01%02@01 16%16@01 07%14 05%24 02%02N00"
   " 01%10 18%01 01%14@01 08%14 03%26 01%02N00 02%12 02@01 11%18 08%40 02%03N00"
   " 03%13 01%02 02%01 02%01@01 01%18 10%37 04%03N00 04%18 01%22 11@01%31 04%07N00 05%39 14%28 07%03N00"
   " 06@01%35 18%25N00 08%32 22%19 02%07N00 11%26 27%15 02@01%09N00"
   " 14%20 11@01%01@01%01 18@01%18 03%03N00 18%15 08%10 20%15 04%01N00 16%36 22%14N00 16%26 02%04 01%03"
   " 22%10 02%03@01N00 21%19 01%06 01%02 26%13@01N00 81%07@01N00";

   //print characters from string (character to print is followed by the number of characters in two-digit format)
   for (int i = 0; i < strlen(dragon); i+=3)
   {
       if (dragon[i] == 32)
       {
           int count = (dragon[i+1] - 48) * 10 + (dragon[i+2] - 48);
           for (int j = 0; j < count; j++){printf(" ");}
       }
       else if (dragon[i] == 64)
       {
       int count = (dragon[i+1] - 48) * 10 + (dragon[i+2] - 48);
       for (int j = 0; j < count; j++){printf("@");}
       }
       else if (dragon[i] == 37)
       {
       int count = (dragon[i+1] - 48) * 10 + (dragon[i+2] - 48);
       for (int j = 0; j < count; j++){printf("%%");}
       }
       else if (dragon[i] == 78){printf("\n");}
       }
}
