## Disclaimer :exclamation:
I am not responsible for the results you may experience using this library. I have not used this API for several years and cannot confirm it fully works. The only function I was able to test the authentication process was the accounts function, which requires an input of XBt. I would honestly love for this library to be used for some high speed trading, I only request that you fork this repository if you are interested in using it. NOTE ALL FUNCTIONS RETURN STRINGS, MUST BE PARSED AS JSON SEPERATELY (NOT INCLUDED IN CODE)

## Importing :vibration_mode:
```c++
#include "bitmex.h"
#include <string>
#include <iostream>

int main()
{
  std::string key = "";
  std::string secret = "";
   
  bitmex bx(key, secret);
  
  std::cout << bx.account("XBt") << std::endl;
  
  return 0;
}
```


![alt](https://github.com/marscolony2040/BitmexCPP/blob/main/img/pic.png)
