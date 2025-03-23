struct s_Out
{
    u_Data Data;
    u_Data Match;
    float Charge;
    float RC;
    bool flg_Use;
};

//Each reference frame is a parallel system state with a shared semiotic database. This allows for multiple parallel operations with the same core neural structure, used for pre-factor analysis, parallel projections, and other things where you want multiple separate operations persistently in parallel.
class c_Time_Series_Reference_Frame
{
public:

    std::vector<std::vector<u_Data>> Input;
    std::vector<std::vector<bool>> Input_Mask;
    std::vector<std::vector<u_Data>> Input_Interm;
    std::vector<std::vector<u_Data>> Output;

    double MSC_APT;
    double Chrono_APT;
    double MSC_MC;
    double Chrono_MC;

    std::vector<std::vector<std::vector<s_Out>>> Bulk;

    int Chrono_Depth;
    int Raw_Depth;

    //Used for the drawdown and comparing Bulk to the current Input_Interm
    int Chrono_Current;

    c_Time_Series_Reference_Frame()
    {
        Chrono_Depth = 0;
        Raw_Depth = 0;
        Chrono_Current = 0;

        MSC_APT = 0.95;
        Chrono_APT = 0.95;
        MSC_MC = 0.95;
        Chrono_MC = 0.95;
    }

    void set_MSC_APT(double p_APT)
    {
        MSC_APT = p_APT;
    }

    void set_Chrono_APT(double p_APT)
    {
        Chrono_APT = p_APT;
    }

    void set_MSC_MC(float p_MC)
    {
        MSC_MC = p_MC;
    }

    void set_Chrono_MC(float p_MC)
    {
        Chrono_MC = p_MC;
    }

    //This sets the first axis of the I/O tables to the depth of Chrono. 
    //You should use the shift_Chrono to read in input you plan on encoding.
    //This is just for initialization in most cases.
    void resize_IO_To_Chrono(int p_Chrono_Depth)
    {
        Chrono_Depth = p_Chrono_Depth;

        Input.resize(p_Chrono_Depth);
        Input_Mask.resize(p_Chrono_Depth);
        Output.resize(p_Chrono_Depth);
    }

    void resize_Raw_Frames(int p_Raw_Depth)
    {
        Raw_Depth = p_Raw_Depth;

        for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
        {
            Input[cou_Chrono].resize(Raw_Depth);
            Input_Mask[cou_Chrono].resize(Raw_Depth);
            Output[cou_Chrono].resize(Raw_Depth);
        }
    }

    std::vector<uint64_t> get_uint_Input(int p_Chrono)
    {
        std::vector<uint64_t> tmp_Data(Input[p_Chrono].size());

        for (int cou_D = 0; cou_D < Input[p_Chrono].size(); cou_D++)
        {
            tmp_Data[cou_D] = Input[p_Chrono][cou_D].U;
        }
        return tmp_Data;
    }

    void copy_Input()
    {
        Input_Interm.clear();

        Input_Interm.resize(Chrono_Depth * 2);

        for (int cou_Chrono = 0; cou_Chrono < Input_Interm.size(); cou_Chrono++)
        {
            Input_Interm[cou_Chrono].resize(Raw_Depth);
        }

        for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
        {
            for (int cou_Index = 0; cou_Index < Raw_Depth; cou_Index++)
            {
                Input_Interm[cou_Chrono][cou_Index].U = 0;
                Input_Interm[cou_Chrono][cou_Index] = Input[cou_Chrono][cou_Index];
            }
        }

        /*
        for (int cou_Chrono = 0; cou_Chrono < Input_Interm.size(); cou_Chrono++)
        {
            std::cout << "\nChrono[" << cou_Chrono << "]";

            for (int cou_Index = 0; cou_Index < Input_Interm[cou_Chrono].size(); cou_Index++)
            {
                std::cout << "\nInterm[" << cou_Chrono << "][" << cou_Index << "] " << Input_Interm[cou_Chrono][cou_Index].D;
            }
        }
        */
    }

    void output_Interm()
    {
        std::cout << "\n\n Interm:";
        for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
        {
            std::cout << "\n [" << cou_Raw << "] ";
            for (int cou_Chrono = 0; cou_Chrono < (Chrono_Depth * 2); cou_Chrono++)
            {
                if ((!(cou_Chrono % Chrono_Depth)) && (cou_Chrono != 0)) { std::cout << " ~ "; }
                std::cout << " " << Input_Interm[cou_Chrono][cou_Raw].D;
            }
        }
    }

    void  move_Interm_To_Output()
    {
        for (int cou_Chrono = Chrono_Depth; cou_Chrono < (Chrono_Depth * 2); cou_Chrono++)
        {
            for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
            {
                Output[cou_Chrono - Chrono_Depth][cou_Raw] = Input_Interm[cou_Chrono][cou_Raw];
            }
        }
    }

    void output_Bulk()
    {
        std::cout << "\n\n Bulk:";
        for (int cou_Chrono = 0; cou_Chrono < Bulk.size(); cou_Chrono++)
        {
            std::cout << "\n    Chrono[" << cou_Chrono << "]";
            for (int cou_Raw = 0; cou_Raw < Bulk[cou_Chrono].size(); cou_Raw++)
            {
                std::cout << "\n       Raw[" << cou_Raw << "]";
                for (int cou_O = 0; cou_O < Bulk[cou_Chrono][cou_Raw].size(); cou_O++)
                {
                    std::cout << "\n          O[" << cou_O << "]";
                    std::cout << " Bulk_Primitive: " << Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
                    std::cout << " Current: " << Input_Interm[Chrono_Current + cou_Chrono][cou_Raw].D;
                    std::cout << " Match: " << Bulk[cou_Chrono][cou_Raw][cou_O].Match.D;
                    std::cout << " Charge: " << Bulk[cou_Chrono][cou_Raw][cou_O].Charge;
                }
            }
        }
        std::cout << "\n\n _~_ Bulk Match:";

        double tmp_Count = 0;

        for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
        {
            std::cout << "\n ___  ___ Raw[" << cou_Raw << "]:";
            for (int cou_O = 0; cou_O < Bulk[0][cou_Raw].size(); cou_O++)
            {
                std::cout << "\n ___  ___  ___ Output[" << cou_O << "]";

                tmp_Count = 0;

                for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
                {
                    tmp_Count += Bulk[cou_Chrono][cou_Raw][cou_O].Match.D;
                }
                std::cout << " Match Count: " << tmp_Count;
            }
        }
    }

    void calculate_Match_Use()
    {
        double tmp_Count = 0;

        for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
        {
            //---std::cout << "\n ___  ___ Raw[" << cou_Raw << "]:";
            for (int cou_O = 0; cou_O < Bulk[0][cou_Raw].size(); cou_O++)
            {
                //---std::cout << "\n ___  ___  ___ Output[" << cou_O << "]";

                tmp_Count = 0;

                for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
                {
                    tmp_Count += Bulk[cou_Chrono][cou_Raw][cou_O].Match.D;
                }
                //---std::cout << " Match Count: " << tmp_Count;

                if (tmp_Count >= 1)
                {
                    for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
                    {
                        Bulk[cou_Chrono][cou_Raw][cou_O].flg_Use = 1;
                    }
                }
            }
        }
    }


    void output_IO()
    {
        for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
        {
            std::cout << "\n\n\n Chrono [" << cou_Chrono << "]: ";
            std::cout << "\n -.- {INPUT} ";
            for (int cou_I = 0; cou_I < Input[cou_Chrono].size(); cou_I++)
            {
                std::cout << "\n -.- -.- <" << cou_I << "> " << Input[cou_Chrono][cou_I].D;
            }
            std::cout << "\n -.- {OUTPUT} ";
            for (int cou_O = 0; cou_O < Output[cou_Chrono].size(); cou_O++)
            {
                std::cout << "\n -.- -.- <" << cou_O << "> " << Output[cou_Chrono][cou_O].D;
            }
        }
    }


    void output_IO_Stats()
    {
        std::cout << "\n IO_Stats:";
        std::cout << "\n ... Chrono_Depth: " << Chrono_Depth;
        for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
        {
            std::cout << "\n ... ... Input_Depth[" << cou_Chrono << "]: " << Input[cou_Chrono].size();
            std::cout << " ... Output_Depth[" << cou_Chrono << "]: " << Output[cou_Chrono].size();
        }
    }

};


















class c_Time_Series_Generator_Module
{
public:

    //The IO, bulk, interm, and other state members.
    std::vector<c_Time_Series_Reference_Frame> RF;

    //Depth of the Chrono array.
    int Chrono_Depth; 

    //The number of inputs to the MSC, the count of raw constructs.
    int Raw_Depth;

    int MSC_Depth;

    NT4::c_Construct_API NT4_Core;

    //Used for the drawdown and comparing Bulk to the current Input_Interm
    int Chrono_Current;

    //One per raw, used to generate the random predictions.
    std::vector<double> Range_High;
    std::vector<double> Range_Low;

    c_Time_Series_Generator_Module()
    {
        Chrono_Depth = 0;
        Raw_Depth = 0;
        MSC_Depth = 0;
        Chrono_Current = 0;
    }

    void init(int p_Chrono_Depth, int p_Raw_Depth, int p_RF_Depth)
    {
        RF.resize(p_RF_Depth);

        /*-*/std::cerr << "\n setup_Higher_Tier_Constructs";
        setup_Higher_Tier_Constructs();

        /*-*/std::cerr << "\n set_Chrono_Depth";
        set_Chrono_Depth(p_Chrono_Depth);

        /*-*/std::cerr << "\n set_Raw_Depth";
        set_Raw_Depth(p_Raw_Depth);

        /*-*/std::cerr << "\n setup_Raw_Tier_Constructs";
        setup_Raw_Tier_Constructs();

        /*-*/std::cerr << "\n update_MSC_Depth";
        update_MSC_Depth();

        /*-*/std::cerr << "\n create_Construct_Connections";
        create_Construct_Connections();

        output_Everything();
    }

    void setup_Higher_Tier_Constructs()
    {
        NT4_Core.register_Construct("Many_To_One", "MSC");

        NT4_Core.register_Construct("Many_To_One", "Chrono");
    }

    void create_Construct_Connections()
    {
        for (int cou_C = 0; cou_C < Raw_Depth; cou_C++)
        {
            NT4_Core.create_Construct_Connection(Raw_Name[cou_C], "MSC");
        }

        NT4_Core.create_Construct_Connection("MSC", "Chrono");

        NT4_Core.output_Constructs();
        NT4_Core.output_Construct_Connections("Chrono");
        NT4_Core.output_Construct_Connections("MSC");
    }

    std::vector<std::string> Raw_Name;

    void setup_Raw_Tier_Constructs()
    {
        Range_Low.resize(Raw_Depth);
        Range_High.resize(Raw_Depth);
        for (int cou_Index = 0; cou_Index < Raw_Depth; cou_Index++)
        {
            std::string tmp_Name = "Raw_" + std::to_string(cou_Index);
            Raw_Name.push_back(tmp_Name);

            NT4_Core.register_Construct("Many_To_One", Raw_Name[cou_Index]);

            Range_Low[cou_Index] = 9999;
            Range_High[cou_Index] = 0;
        }
        NT4_Core.output_Constructs();
    }

    //Sets the Chrono depth.
    void set_Chrono_Depth(int p_Depth)
    {
        Chrono_Depth = p_Depth;

        resize_IO_To_Chrono();
    }

    void set_MSC_APT(int p_RF, float p_APT)
    {
        RF[p_RF].set_MSC_APT(p_APT);
    }

    void set_Chrono_APT(int p_RF, float p_APT)
    {
        RF[p_RF].set_Chrono_APT(p_APT);
    }

    void set_MSC_MC(int p_RF, float p_MC)
    {
        RF[p_RF].set_MSC_MC(p_MC);
    }

    void set_Chrono_MC(int p_RF, float p_MC)
    {
        RF[p_RF].set_Chrono_MC(p_MC);
    }

    //This sets the first axis of the I/O tables to the depth of Chrono. 
    //You should use the shift_Chrono to read in input you plan on encoding.
    //This is just for initialization in most cases.
    void resize_IO_To_Chrono()
    {
        for (int cou_S = 0; cou_S < RF.size(); cou_S++)
        {
            RF[cou_S].resize_IO_To_Chrono(Chrono_Depth);
        }

        std::vector<uint64_t> tmp_Gathered_Chrono(Chrono_Depth);

        NT4_Core.set_Input_uint("Chrono", Chrono_Depth, (tmp_Gathered_Chrono.data()));
    }

    void resize_Raw_Frames()
    {
        for (int cou_S = 0; cou_S < RF.size(); cou_S++)
        {
            RF[cou_S].resize_Raw_Frames(Raw_Depth);
        }
    }

    void set_Raw_Depth(int p_Raw_Depth)
    {
        Raw_Depth = p_Raw_Depth;

        resize_Raw_Frames();
    }

    void shift_Data(int p_RF = 0, float p_Init_Value = 0)
    {
        //---validate_RF(p_RF);

        for (int cou_Chrono = 0; cou_Chrono < (Chrono_Depth - 1); cou_Chrono++)
        {
            RF[p_RF].Input[cou_Chrono] = RF[p_RF].Input[cou_Chrono + 1];
            RF[p_RF].Input_Mask[cou_Chrono] = RF[p_RF].Input_Mask[cou_Chrono + 1];
            RF[p_RF].Output[cou_Chrono] = RF[p_RF].Output[cou_Chrono + 1];
        }
        RF[p_RF].Input[(Chrono_Depth - 1)].clear();
        RF[p_RF].Input[(Chrono_Depth - 1)].resize(Raw_Depth);

        RF[p_RF].Input_Mask[(Chrono_Depth - 1)].clear();
        RF[p_RF].Input_Mask[(Chrono_Depth - 1)].resize(Raw_Depth);

        RF[p_RF].Output[(Chrono_Depth - 1)].clear();
        RF[p_RF].Output[(Chrono_Depth - 1)].resize(Raw_Depth);

        if (p_Init_Value != 0)
        {
            for (int cou_R = 0; cou_R < Raw_Depth; cou_R++)
            {
                RF[p_RF].Input[(Chrono_Depth - 1)][cou_R].D = p_Init_Value;
            }
        }
    }

    void set_Input_Index(int p_RF, int p_Index, uint64_t p_Data)
    {
        //---validate_RF(p_RF);

        RF[p_RF].Input[Chrono_Depth - 1][p_Index].U = p_Data;
        RF[p_RF].Input_Mask[Chrono_Depth - 1][p_Index] = 1;

        u_Data tmp_Bit;
        tmp_Bit.U = 0;
        tmp_Bit.U = p_Data;

        if (tmp_Bit.D > Range_High[p_Index]) { Range_High[p_Index] = tmp_Bit.D; }
        if (tmp_Bit.D < Range_Low[p_Index]) { Range_Low[p_Index] = tmp_Bit.D; }
    }

    void set_Input_Index(int p_RF, int p_Index, double p_Data)
    {
        //---validate_RF(p_RF);

        RF[p_RF].Input[Chrono_Depth - 1][p_Index].D = p_Data;
        RF[p_RF].Input_Mask[Chrono_Depth - 1][p_Index] = 1;

        if (p_Data > Range_High[p_Index]) { Range_High[p_Index] = p_Data; }
        if (p_Data < Range_Low[p_Index]) { Range_Low[p_Index] = p_Data; }

        //std::cout << "\n set_Input_Index(p_RF:" << p_RF << ", p_Index: " << p_Index << ", p_Data: " << p_Data;
    }

    u_Data get_Output_Index(int p_RF, int p_Index)
    {
        //---validate_RF(p_RF);

        return RF[p_RF].Output[0][p_Index];
    }

    std::vector<std::vector<std::vector<NT4::s_Out>>> get_Bulk(int p_RF)
    {
        return RF[p_RF].Bulk;
    }



    void update_MSC_Depth()
    {
        std::vector<uint64_t> tmp_Gathered_MSC(Raw_Depth);
        MSC_Depth = Raw_Depth;
        NT4_Core.set_Input_uint("MSC", Raw_Depth, (tmp_Gathered_MSC.data()));
    }

    std::vector<uint64_t> get_uint_Input(int p_RF, int p_Chrono)
    {
        //---validate_RF(p_RF);

        return RF[p_RF].get_uint_Input(p_Chrono);
    }

    //First we do the lower ones, then from those we gather the treetops to bind together into the MSC, which then feeds into the Chrono.
    void encode(int p_RF)
    {
        //---validate_RF(p_RF);

        for (int cou_Chrono = 0; cou_Chrono < (Chrono_Depth); cou_Chrono++)
        {
            //---std::cout << "\n _~_~_ Chrono[ " << cou_Chrono << "] _~_~_";
            for (int cou_Index = 0; cou_Index < Raw_Depth; cou_Index++)
            {
                NT4_Core.set_Input_uint(Raw_Name[cou_Index], 1, &(RF[p_RF].Input[cou_Chrono][cou_Index].U));
                NT4_Core.encode(Raw_Name[cou_Index]);
                //---std::cout << "\n Raw[" << Raw_Name[cou_Index] << "] ";
                //---NT4_Core.output_Scaffold(Raw_Name[cou_Index]);
                //---NT4_Core.output_Scaffold_Symbols_Float(Raw_Name[cou_Index]);
            }
            NT4_Core.round_Up_Input("MSC");
            //---std::cout << "\n MSC Input: ";
            //---NT4_Core.output_Input_uint("MSC");
            NT4_Core.encode("MSC");
            //---std::cout << "\n MSC Scaffold: ";
            //---NT4_Core.output_Scaffold_Symbols_uint("MSC");
            NT4_Core.pull_Chrono_From_Lower_Connection("Chrono");
        }
        NT4_Core.encode("Chrono");
        //---std::cout << "\n Chrono Scaffold: ";
        //---NT4_Core.output_Scaffold("Chrono");
    }

    void output_Scaffolds()
    {
        std::cout << "\n\n There are [" << Chrono_Depth << "] frames in the time series. Each frame has [" << Raw_Depth << "] raw tier constructs, one multi-sensory-construct (MSC), and one Chrono construct on top to handle the time series. This shows the most recent frame as it is the only one with a scaffold built.";
        
        std::cout << "\n\n";
        std::cout << "\n _ __  ___   ____    _____    ____   ___  __ _";
        std::cout << "\n _ __  ___   ____    _____    ____   ___  __ _";
        std::cout << "\n _ __  ___   ____    _____    ____   ___  __ _";
        for (int cou_Index = 0; cou_Index < Raw_Depth; cou_Index++)
        {
            std::cout << "\n\n -Raw_Tier_Scaffold[" << cou_Index << "] in Chrono[0]";
            std::cout << "\n\n --- Scaffold as adresses:";
            NT4_Core.output_Scaffold(Raw_Name[cou_Index]);
            std::cout << "\n\n --- Scaffold as floats:";
            NT4_Core.output_Scaffold_Symbols_Float(Raw_Name[cou_Index]);
            //std::cout << Raw_Name[cou_Index];
        }

        std::cout << "\n\n - MSC Scaffold:";
        NT4_Core.output_Scaffold("MSC");

        std::cout << "\n\n - Chrono Scaffold: ";
        NT4_Core.output_Scaffold("Chrono");
        std::cout << "\n\n";
    }

    void copy_Input(int p_RF)
    {
        //---validate_RF(p_RF);

        RF[p_RF].copy_Input();
    }

    void output_Interm(int p_RF)
    {
        //---validate_RF(p_RF);

        RF[p_RF].output_Interm();
    }

    void  move_Interm_To_Output(int p_RF)
    {
        //---validate_RF(p_RF);

        RF[p_RF].move_Interm_To_Output();
    }


    //Encodes an input set.
    void eval(int p_RF)
    {
        //---validate_RF(p_RF);
        NT4_Core.set_Action_Potential_Threshold("Chrono", float(RF[p_RF].Chrono_APT));
        NT4_Core.set_Action_Potential_Threshold("MSC", float(RF[p_RF].MSC_APT));
        NT4_Core.set_Modifier_Charge("Chrono", float(RF[p_RF].Chrono_MC));
        NT4_Core.set_Modifier_Charge("MSC", float(RF[p_RF].MSC_MC));

        copy_Input(p_RF);

        //---output_IO();

        query_Arrays(p_RF);

        //system("PAUSE");

        //---output_Interm();

        //drawdown_Arrays();

        //generate_Avg_Output();

        //record_Current_Projection();
        //record_Current_IO();

        //---std::cout << "\n\n\n [[[ Historical_Prediction ]]]:\n\n";

        //output_Historical_Projections();
    }

    double get_Input(int p_RF, int p_Chrono, int p_Index)
    {
        return RF[p_RF].Input[p_Chrono][p_Index].D;
    }

    void query_Arrays(int p_RF)
    {
        std::vector<uint64_t> tmp_Chron(Chrono_Depth);
        NT4_Core.set_Input_uint("Chrono", Chrono_Depth, tmp_Chron.data());

        //---std::cout << "\n\n\n query_Array()"; std::cout.flush();
        //---std::cout << "\n Chrono_Depth: " << Chrono_Depth; std::cout.flush();
        for (int cou_Step = 0; cou_Step < Chrono_Depth; cou_Step++)
        {
            //---output_Interm(p_RF); std::cout.flush();

            NT4_Core.reset_Output("Chrono");
            NT4_Core.gather_Treetops("Chrono");

            Chrono_Current = cou_Step;
            RF[p_RF].Chrono_Current = cou_Step;

            //---std::cout << "\n -+- Step[" << cou_Step << "]"; std::cout.flush();
            //[0] is the newest so we read the chrono in the same way it appears, oldest to newest.
            for (int cou_Chrono = 0; cou_Chrono < (Chrono_Depth - 1); cou_Chrono++)
            {
                //---std::cout << "\n -+- -+- Chrono[" << cou_Chrono << "]"; std::cout.flush();
                for (int cou_Index = 0; cou_Index < Raw_Depth; cou_Index++)
                {
                    //---std::cout << "\n -+- -+- -+- Index[" << cou_Index << "]"; std::cout.flush();
                    //---std::cout << "\n -+- -+- -+- -+- Input_Interm[" << (cou_Step + cou_Chrono) << "][" << cou_Index << "]: " << (RF[p_RF].Input_Interm[cou_Step + cou_Chrono][cou_Index].D); std::cout.flush();
                    u_Data tmp_Bit;
                    tmp_Bit.U = 0;
                    tmp_Bit.U = RF[p_RF].Input_Interm[cou_Step + cou_Chrono][cou_Index].U;
                    
                    if ((tmp_Bit.D - int(tmp_Bit.D)) < 0.5)
                    {
                        tmp_Bit.D = int(tmp_Bit.D);
                    }
                    if ((tmp_Bit.D - int(tmp_Bit.D)) >= 0.5)
                    {
                        tmp_Bit.D = int(tmp_Bit.D) + 1;
                    }

                    NT4_Core.set_Input_uint(Raw_Name[cou_Index], 1, &(tmp_Bit.U));
                    NT4_Core.check_Symbol(Raw_Name[cou_Index]);
                }

                NT4_Core.reset_Output("MSC");
                NT4_Core.gather_Treetops("MSC");
                NT4_Core.round_Up_Input("MSC");
                //---std::cout << "\n MSC Input: ";
                //---NT4_Core.output_Input_uint("MSC");
                NT4_Core.query_Spacial("MSC");
                //---std::cout << "\n MSC Scaffold: ";
                //---NT4_Core.output_Scaffold("MSC");
                NT4_Core.gather_Treetops("MSC");
                //---std::cout << "\n MSC Output_uint: ";
                //---NT4_Core.output_Output_uint("MSC");

                NT4_Core.pull_From_Lower_Connections("Chrono");
                NT4_Core.query_Given_Index("Chrono", cou_Chrono);
                //---std::cout << "\n Chrono[" << cou_Chrono << "]: ";
                //---NT4_Core.output_Input_uint("Chrono");
                //---NT4_Core.output_Scaffold("Chrono");
            }
            NT4_Core.gather_Treetops("Chrono");

            //---std::cout << "\n Output Output_uint Chrono: ";
            //---NT4_Core.output_Output_uint("Chrono");

            drawdown_Arrays(p_RF);

            calculate_Match_Use(p_RF);

            generate_Avg_Output(p_RF, cou_Step, (cou_Step + Chrono_Depth));
        }
        //---output_Bulk(p_RF);

        move_Interm_To_Output(p_RF);
    }

    void drawdown_Arrays(int p_RF)
    {
        //---validate_RF(p_RF);

        //std::cout << "\n lkdsjflksjfpjpoijewopjwiojoi  eerwf hthjtr hrt t  jf kljfjejw;jo4ij93ufj043j0     0000";

        RF[p_RF].Bulk.resize(Chrono_Depth);

        for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
        {
            RF[p_RF].Bulk[cou_Chrono].resize(Raw_Depth);

            NT4_Core.reset_Output("MSC");
            NT4_Core.pull_From_Upper_Index("MSC", "Chrono", cou_Chrono);


            //std::cout << "\n lkdsjflksjfpjpoijewopjwiojoi  eerwf hthjtr hrt t  jf kljfjejw;jo4ij93ufj043j0     1111";

            for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
            {
                NT4_Core.reset_Output(Raw_Name[cou_Raw]);
                NT4_Core.pull_From_Upper_Index(Raw_Name[cou_Raw], "MSC", cou_Raw);

                RF[p_RF].Bulk[cou_Chrono][cou_Raw].resize(NT4_Core.get_Output_Depth(Raw_Name[cou_Raw]));

                for (int cou_O = 0; cou_O < NT4_Core.get_Output_Depth(Raw_Name[cou_Raw]); cou_O++)
                {
                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Data.U = 0;
                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Data.U = NT4_Core.get_Output_Primitive_uint(Raw_Name[cou_Raw], cou_O);

                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Match.D = 0.0;
                    if (RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Data.D == RF[p_RF].Input_Interm[Chrono_Current + cou_Chrono][cou_Raw].D)
                    {
                        //---std::cout << "  << Match >>";
                        RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Match.D = 1.0;
                    }

                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Charge = 0.0;
                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].Charge = NT4_Core.get_Output_Charge(Raw_Name[cou_Raw], cou_O);

                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].flg_Use = 1;
                    RF[p_RF].Bulk[cou_Chrono][cou_Raw][cou_O].RC = NT4_Core.get_Output_RC(Raw_Name[cou_Raw], cou_O);
                }
                //---std::cout << "\n\n Raw[" << cou_Raw << "]: ";
                //---NT4_Core.output_Output_Double(Raw_Name[cou_Raw]);
            }
        }

        //std::cout << "\n lkdsjflksjfpjpoijewopjwiojoi  eerwf hthjtr hrt t  jf kljfjejw;jo4ij93ufj043j0     3333";
    }

    //WIP
    void generate_Avg_Output(int p_RF, int p_Chrono, int p_Interm)
    {
        std::vector<uint64_t> tmp_Pattern;
        u_Data tmp_Bit;

        double tmp_Total = 0.0;
        float tmp_Count = 0.0;

        //---std::cout << "\n\n\n  --==<<                                >>==-- ";
        //---std::cout << "\n  --==<<      Generating Avg Output     >>==-- ";
        //---std::cout << "\n  --==<<                                >>==-- \n\n\n\n";

        //---std::cout << "\n\n #Outputs: " << NT4_Core.get_Output_Depth("Chrono");

        for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
        {
            //if (!(cou_Raw % 5)) { std::cout << "\n"; }

            //---std::cout << "\n    <[R " << cou_Raw << " ";
            //NT4_Core.output_Output_Double(Raw_Name[cou_Raw]);

            tmp_Total = 0.0;
            tmp_Count = 0.0;

            //---std::cout << " CBits: ";
            for (int cou_O = 0; cou_O < NT4_Core.get_Output_Depth(Raw_Name[cou_Raw]); cou_O++)
            {
                tmp_Pattern = NT4_Core.get_Output_Pattern_uint(Raw_Name[cou_Raw], cou_O);
                tmp_Bit.U = 0;
                tmp_Bit.U = tmp_Pattern[0];
                tmp_Total += tmp_Bit.D;
                tmp_Count++;
                //---std::cout << " " << cou_O << "{" << tmp_Bit.D;

            }

            //---std::cout << "\n BEFORE::::::::::::::::::::::::::::::::::::::: tmp_Total: " << tmp_Total << " tmp_Count: " << tmp_Count;
            if ((tmp_Count == 0) || (tmp_Total == 0))
            {
                tmp_Total = 0;
            }
            else
            {
                tmp_Total = tmp_Total / tmp_Count;
            }

            double tmp_Remainder = (tmp_Total - int(tmp_Total));

            int tmp_Total_Rounded = int(tmp_Total);
            if (tmp_Remainder > 0.5) { tmp_Total_Rounded++; }

            //---std::cout << "\n AFTER::::::::::::::::::::::::::::::::::::::: tmp_Total: " << tmp_Total << " tmp_Count: " << tmp_Count;
            RF[p_RF].Output[Chrono_Depth - 1][cou_Raw].D = tmp_Total_Rounded;

            if (p_Interm != -1)
            {
                RF[p_RF].Input_Interm[p_Interm][cou_Raw].D = tmp_Total_Rounded;
            }

            //---std::cout << " Ttl: " << tmp_Total;
            //---std::cout << " TtlR: " << tmp_Total_Rounded;
            //---std::cout << " ]> ";
        }
    }

    void output_Bulk(int p_RF)
    {
        //---validate_RF(p_RF);

        RF[p_RF].output_Bulk();
    }

    void calculate_Match_Use(int p_RF)
    {
        //---validate_RF(p_RF);

        RF[p_RF].calculate_Match_Use();
    }

    std::vector<u_Data> get_Random_Raw_Projection(int p_Raw)
    {
        std::vector<u_Data> tmp_Data;
        tmp_Data.resize(Chrono_Depth);

        //std::cout << "\n Random Projection[" << p_Raw << "] Low: " << Range_Low[p_Raw] << " High: " << Range_High[p_Raw];

        int tmp_Diff = int(Range_High[p_Raw] - Range_Low[p_Raw]);

        for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
        {
            if (tmp_Diff == 0)
            {
                tmp_Data[cou_Chrono].D = Range_High[p_Raw];
            }
            else
            {
                tmp_Data[cou_Chrono].D = (rand() % tmp_Diff) + Range_Low[p_Raw];
            }
        }
        return tmp_Data;
    }

    void output_IO(int p_RF)
    { 
        //---validate_RF(p_RF);

        RF[p_RF].output_IO();
    }

    void output_IO_Stats(int p_RF)
    {
        //---validate_RF(p_RF);

        RF[p_RF].output_IO_Stats();
    }

    void output_Everything()
    {
        NT4_Core.output_Constructs();

        for (int cou_RF = 0; cou_RF < RF.size(); cou_RF++)
        {
            //output_IO(cou_RF);
            output_IO_Stats(cou_RF);
        }
    }
};
