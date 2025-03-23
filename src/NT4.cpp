// NT4.cpp : Defines the entry point for the application.
//



#include "NT4.h"
#include "include/HomeoStasis/c_Granulator.h"
#include "include/HomeoStasis/c_AE_Interface.h"
#include "include/c_Gaia.h"

using namespace std;

//Config.ssv - This file configures the network and sets the initial hyper-parameters. It handles the loading of any node files needed when using pretrained models.

//Input.ssv - This file holds the input data, each row an index

//Output.ssv - This file holds the output data, a table holding traces.

int main(int argc, char** argv)
{
    c_GaiaOS_Text_Server GaiaOS;

    GaiaOS.run();

    return 1;
}
