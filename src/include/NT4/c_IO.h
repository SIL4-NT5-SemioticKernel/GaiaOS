


//This class serves as the user interface for the actuator I/O
class c_IO_Set
{
public:

    //Data is stored as an array of uint64_t
    uint64_t* Data;

    //The depth of the data.
    int Depth;

    c_IO_Set()
    {
        Data = NULL;

        Depth = 0;
    }

    void set_Depth(int p_Depth)
    {
        //If Data is NULL then everything else should be as well.
        if (Data != NULL)
        {
            delete[] Data;
            Data = NULL;
        }

        Depth = p_Depth;

        if (Depth == 0) { Data = NULL; return; }

        Data = new uint64_t[Depth];

        wipe_Data();
    }


    //Reset it if need be.
    void reset()
    {
        if (Data != NULL)
        {
            delete[] Data;
            Data = NULL;
            Depth = 0;
        }
    }

    //This shifts the output from current to 0.
    void wipe_Data()
    {
        for (int cou_Index = 0; cou_Index < Depth; cou_Index++)
        {
            Data[cou_Index] = 0;
        }
    }

    //Accepts an input and updates the correct index with it
    void set_Value(uint64_t p_Value, int p_Index = -1) //p_Index comes after since it may be 0
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }

        //First get the concrete value.
        Data[p_Index] = p_Value;
    }

    //Accepts an array, resets the data, then reads the array into the data then sets the depth.
    void set_Array_Value(uint64_t* p_Data_Array, int p_Depth)
    {
        reset();

        //Initialize the wiped input array and copy the data to it from the arguments.
        Depth = p_Depth;
        Data = new uint64_t[Depth];
        for (int cou_Index = 0; cou_Index < Depth; cou_Index++)
        {
            Data[cou_Index] = p_Data_Array[cou_Index];
        }
    }

    //Returns the value.
    uint64_t get_Value(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }

        return Data[p_Index];
    }

    //This shifts the output from current to 0.
    void shift_Data()
    {
        //Shift each array one index towards [0].
        for (int cou_Index = 0; cou_Index < (Depth - 1); cou_Index++)
        {
            Data[cou_Index] = Data[cou_Index + 1];
        }

        //Set the current to 0.0 in preparation for input.
        Data[Depth - 1] = 0;
    }

    //Outputs the data
    void output()
    {
        for (int cou_Index = 0; cou_Index < Depth; cou_Index++)
        {
            std::cout << "\n[" << cou_Index << "] Data: " << Data[cou_Index];
        }
    }
};





//This class serves as the user interface for the actuator I/O
class c_2D_IO_Set
{
public:

    //Data is stored as an array of uint64_t
    uint64_t** Data;

    //The depth of the data.
    //[0] == x
    //[1] == y
    //[] Those are arbitrarily chosen but if most follow them it should be easy to share work.
    int Depth[2];

    c_2D_IO_Set()
    {
        Data = NULL;

        Depth[0] = 0;
        Depth[1] = 0;
    }

    void set_Depth(int p_X_Depth, int p_Y_Depth)
    {
        reset();

        Depth[0] = p_X_Depth;
        Depth[1] = p_Y_Depth;

        Data = new uint64_t*[Depth[0]];

        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            Data[cou_X] = new uint64_t [Depth[1]];

            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                Data[cou_X][cou_Y] = 0;
            }
        }
    }


    //Reset it if need be.
    void reset()
    {
        //If Data is NULL then everything else should be as well.
        //If Data is not NULL then there should be tiers.
        if (Data != NULL)
        {
            for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
            {
                if (Data[cou_X] != NULL)
                {
                    delete[] Data[cou_X];
                    Data[cou_X] = NULL;
                }
            }

            delete[] Data;
            Data = NULL;
        }
        Depth[0] = 0;
        Depth[1] = 0;
    }

    //This shifts the output from current to 0.
    void wipe_Data()
    {
        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                Data[cou_X][cou_Y] = 0;
            }
        }

    }

    //Accepts an input and updates the correct index with it
    void set_Value(uint64_t p_Value, int p_X, int p_Y) //p_Index comes after since it may be 0
    {
        //First get the concrete value.
        Data[p_X][p_Y] = p_Value;
    }

    //Accepts an array, resets the data, then reads the array into the data then sets the depth.
    void set_Array_Value(uint64_t** p_Data_Array, int p_X_Depth, int p_Y_Depth)
    {
        reset();

        set_Depth(p_X_Depth, p_Y_Depth);

        //Initialize the wiped input array and copy the data to it from the arguments.
        for (int cou_X=0;cou_X<Depth[0];cou_X++)
        {
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                Data[cou_X][cou_Y] = p_Data_Array[cou_X][cou_Y];
            }
        }
    }

    //Returns the value.
    uint64_t get_Value(int p_X, int p_Y)
    {
        return Data[p_X][p_Y];
    }

    //This shifts the output from current to 0 for Chrono.
    //We only shift the X-axis no matter the dimension. A line inches along, an image scrolls, a cube slides, etc.
    void shift_Data()
    {
        //Shift each array one index towards [0].
        for (int cou_X = 0; cou_X < (Depth[0] - 1); cou_X++)
        {
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                Data[cou_X][cou_Y] = Data[cou_X + 1][cou_Y];
            }
        }
        for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
        {
            Data[Depth[0] - 1][cou_Y] = 0;
        }
    }

    //Outputs the data
    void output()
    {
        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            std::cout << "\n";
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                std::cout << " [" << cou_X << "][" << cou_Y << "]: " << Data[cou_X][cou_Y];
            }
        }
    }
};










//This class serves as the user interface for the actuator I/O
class c_3D_IO_Set
{
public:

    //Data is stored as an array of uint64_t
    uint64_t*** Data;

    //The depth of the data.
    //[0] == x
    //[1] == y
    //[2] == z
    //[] Those are arbitrarily chosen but if most follow them it should be easy to share work.
    int Depth[3];

    c_3D_IO_Set()
    {
        Data = NULL;

        Depth[0] = 0;
        Depth[1] = 0;
        Depth[2] = 0;
    }

    void set_Depth(int p_X_Depth, int p_Y_Depth, int p_Z_Depth)
    {
        reset();

        Depth[0] = p_X_Depth;
        Depth[1] = p_Y_Depth;
        Depth[2] = p_Z_Depth;

        Data = new uint64_t ** [Depth[0]];

        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            Data[cou_X] = new uint64_t * [Depth[1]];

            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                Data[cou_X][cou_Y] = new uint64_t [Depth[2]];

                for (int cou_Z = 0; cou_Z < Depth[2]; cou_Z++)
                {
                    Data[cou_X][cou_Y][cou_Z] = 0;
                }
            }
        }
    }


    //Reset it if need be.
    void reset()
    {
        //If Data is NULL then everything else should be as well.
        //If Data is not NULL then there should be tiers.
        if (Data != NULL)
        {
            for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
            {
                if (Data[cou_X] != NULL)
                {
                    for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
                    {
                        if (Data[cou_X][cou_Y] != NULL)
                        {
                            delete[] Data[cou_X][cou_Y];
                            Data[cou_X][cou_Y] = NULL;
                        }
                    }
                    delete[] Data[cou_X];
                    Data[cou_X] = NULL;
                }
            }

            delete[] Data;
            Data = NULL;
        }
        Depth[0] = 0;
        Depth[1] = 0;
        Depth[2] = 0;
    }

    //This shifts the output from current to 0.
    void wipe_Data()
    {
        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                for (int cou_Z = 0; cou_Z < Depth[2]; cou_Z++)
                {
                    Data[cou_X][cou_Y][cou_Z] = 0;
                }
            }
        }

    }

    //Accepts an input and updates the correct index with it
    void set_Value(uint64_t p_Value, int p_X, int p_Y, int p_Z)
    {
        //First get the concrete value.
        Data[p_X][p_Y][p_Z] = p_Value;
    }

    //Accepts an array, resets the data, then reads the array into the data then sets the depth.
    void set_Array_Value(uint64_t*** p_Data_Array, int p_X_Depth, int p_Y_Depth, int p_Z_Depth)
    {
        reset();

        set_Depth(p_X_Depth, p_Y_Depth, p_Z_Depth);

        //Initialize the wiped input array and copy the data to it from the arguments.
        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                for (int cou_Z = 0; cou_Z < Depth[1]; cou_Z++)
                {
                    Data[cou_X][cou_Y][cou_Z] = p_Data_Array[cou_X][cou_Y][cou_Z];
                }
            }
        }
    }

    //Returns the value.
    uint64_t get_Value(int p_X, int p_Y, int p_Z)
    {
        return Data[p_X][p_Y][p_Z];
    }

    //This shifts the output from current to 0 for Chrono.
    //We only shift the X-axis no matter the dimension. A line inches along, an image scrolls, a cube slides, etc.
    void shift_Data()
    {
        //Shift each array one index towards [0].
        for (int cou_X = 0; cou_X < (Depth[0] - 1); cou_X++)
        {
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                for (int cou_Z = 0; cou_Z < Depth[2]; cou_Z++)
                {
                    Data[cou_X][cou_Y][cou_Z] = Data[cou_X + 1][cou_Y][cou_Z];
                }
            }
        }
        for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
        {
            for (int cou_Z = 0; cou_Z < Depth[2]; cou_Z++)
            {
                Data[Depth[0] - 1][cou_Y][cou_Z] = 0;
            }
        }
    }

    //Outputs the data
    void output()
    {
        for (int cou_X = 0; cou_X < Depth[0]; cou_X++)
        {
            std::cout << "\n";
            for (int cou_Y = 0; cou_Y < Depth[1]; cou_Y++)
            {
                std::cout << "\n";
                for (int cou_Z = 0; cou_Z < Depth[2]; cou_Z++)
                {
                    std::cout << " [" << cou_X << "][" << cou_Y << "][" << cou_Z << "]: " << Data[cou_X][cou_Y][cou_Z];
                }
            }
        }
    }
};