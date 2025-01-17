
class c_Data
{
public:
    u_Data Union;

    c_Data()
    {
        Union.V = NULL;
    }

    void set_uint64_t(uint64_t p_Data)
    {
        Union.U = p_Data;
    }

    void set_double(double p_Data)
    {
        Union.D = p_Data;
    }

    void set_voidstar(void * p_Data)
    {
        Union.V = p_Data;
    }

    uint64_t get_uint64_t() { return Union.U; }
    double get_double() { return Union.D; }
    void* get_voidstar() { return Union.V; }
};

//This class serves as the user interface for the goal input
//It encapsulates the concrete data, granulated data, delta, and the goal w/ deviation.
//Contains the settings such as the granulation filter, and the data table for the input (1D string of data)
//May implement templates later.
class c_AE_IO_Element
{
public:

    //The granulation portion. When an input granularity is needed this calls.
    c_Granulator Granulator;

    //Concrete data is stored internally as an array of double.
    c_Data* Data;

    //Granulated Data & Deltas are stored here at the same depth as the Data.
    c_Data* Granulated;
    c_Data* Delta;

    //The deviation state.
    std::vector<float> Deviation;

    //The current deviation from the goal concrete, and the deviation granulated.
    float Goal_Deviation;

    //The depth of the data.
    int Depth;

    c_AE_IO_Element()
    {
        Data = NULL;
        Granulated = NULL;
        Delta = NULL;

        Depth = 0;

        Goal_Deviation = 0;
    }

    void set_Depth(int p_Depth)
    {
        //If Data is NULL then everything else should be as well.
        if (Data != NULL)
        {
            delete[] Data;
            Data = NULL;

            delete[] Granulated;
            Granulated = NULL;

            delete[] Delta;
            Delta = NULL;
        }

        Depth = p_Depth;

        Data = new c_Data[Depth];
        Granulated = new c_Data[Depth];
        Delta = new c_Data[Depth];

        Deviation.resize(Depth);

        wipe_Data();
    }


    //This shifts the output from current to 0.
    void wipe_Data()
    {
        for (int cou_Index = 0; cou_Index < Depth; cou_Index++)
        {
            Data[cou_Index].set_voidstar(NULL);
            Granulated[cou_Index].set_voidstar(NULL);
            Delta[cou_Index].set_voidstar(NULL);
        }
    }

    //When adding granulations start at the goal range and work outwards.
    void add_Granulation(double p_Bottom, double p_Top)
    {
        Granulator.add_Var(p_Bottom, p_Top);
    }

    //Calculates the goal for a given index.
    void calculate_Deviation(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }

        Deviation[p_Index] = 0;
        
        if (Granulated[p_Index].get_double() > 0) { Deviation[p_Index] = Granulated[p_Index].get_double() * -1; }
        if (Granulated[p_Index].get_double() < 0) { Deviation[p_Index] = Granulated[p_Index].get_double(); }
        
        if (Granulated[p_Index].get_double() > 0) { Deviation[p_Index] = -1; }
        if (Granulated[p_Index].get_double() < 0) { Deviation[p_Index] = 1; }
    }

    //This gets the delta of the variable at a given index. It cannot calculate the index 0 as there is no data to compare too previously.
    //When the array is shifted the deltas are copied, this way we can still record deltas that fall off from [0]. 
    //If no p_Index is specified it calculates the latest, used after shifting.
    //Assumes index exists.
    void calculate_Delta(int p_Index = -1)
    {
        //If no index is selected it assumes the current index and adjusts accordingly.
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }

        if (Data[Depth - 1].get_double() < Data[Depth - 2].get_double()) { Delta[Depth - 1].set_double(-1); }
        if (Data[Depth - 1].get_double() == Data[Depth - 2].get_double()) { Delta[Depth - 1].set_double(0); }
        if (Data[Depth - 1].get_double() > Data[Depth - 2].get_double()) { Delta[Depth - 1].set_double(1); }
    }

    //Accepts an input and updates the correct index with it, then granulates it, then deltatizes it.
    void set_Value(double p_Value, int p_Index = -1) //p_Index comes after since it may be 0
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }

        //First get the concrete value.
        Data[p_Index].set_double(p_Value);

        //Then the granulated value.
        Granulated[p_Index].set_double(Granulator.get_Granulated(p_Value));

        //The the delta.
        calculate_Delta(p_Index);

        //Then the deviation.
        calculate_Deviation(p_Index);
    }

    //Returns the value.
    double get_Value_Data(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }
    
        return Data[p_Index].get_double();
    }

    //Returns the value.
    double get_Value_Granulated(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }
    
        return Granulated[p_Index].get_double();
    }

    //Returns the value.
    double get_Value_Delta(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }
    
        return Delta[p_Index].get_double();
    }

    float get_Value_Deviation(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }

        return Deviation[p_Index];
    }

    //Returns the value.
    uint64_t get_Value_Data_uint64_t(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }
    
        return Data[p_Index].get_uint64_t();
    }

    //Returns the value.
    uint64_t get_Value_Granulated_uint64_t(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }
    
        return Granulated[p_Index].get_uint64_t();
    }

    //Returns the value.
    uint64_t get_Value_Delta_uint64_t(int p_Index = -1)
    {
        if (p_Index == -1)
        {
            p_Index = Depth - 1;
        }
    
        return Delta[p_Index].get_uint64_t();
    }

    //This shifts the output from current to 0.
    void shift_Data()
    {
        //Shift each array one index towards [0].
        for (int cou_Index = 0; cou_Index < (Depth - 1); cou_Index++)
        {
            Data[cou_Index] = Data[cou_Index + 1];
            Granulated[cou_Index] = Granulated[cou_Index + 1];
            Delta[cou_Index] = Delta[cou_Index + 1];
            Deviation[cou_Index] = Deviation[cou_Index + 1];
        }

        //Set the current to 0.0 in preparation for input.
        Data[Depth - 1].set_double(0.0f);
        Granulated[Depth - 1].set_double(0.0f);
        Delta[Depth - 1].set_double(0.0f);
        Deviation[Depth - 1] = 0.0f;
    }

    //Outputs the data
    void output()
    {
        for (int cou_Index = 0; cou_Index < Depth; cou_Index++)
        {
            std::cout << "\n[" << cou_Index << "] Data: " << Data[cou_Index].get_double() << " << Granulated: " << Granulated[cou_Index].get_double() << " << Delta: " << Delta[cou_Index].get_double() << " << Deviation: " << Deviation[cou_Index];
        }
    }
};

