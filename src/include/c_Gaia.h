//So:
//This is an exploratory build, not to be part of the safety build. This is being rewritten for embedded hardware during the left v-leg of a certifiable build. This is to be used on systems that don't require certification, R&D is the expected main use case.

//Naming changes planned:
//GaiaOS is going to be changed, to what is still in debate, but GaiaOS already exists as a business.
//Homeostsis module will be changed to goal_seeker_module


class c_Homeostasis_IO_Module
{
public:

    //The count.
    int Afferent_Count; //Variables dependent on this node: Afferent[]
    int Efferent_Count; //Variables dependent on this node: Efferent[]

    //One input for each index.
    c_AE_IO_Element** Afferent;
    c_AE_IO_Element** Efferent;

    //The array to hold the gathered input, and the count of elements in it.
    uint64_t* Gathered_Afferent[3]; //[0]: Concrete, [1]: Granulated, [2]: Delta
    uint64_t* Gathered_Efferent; //This is separate because it can have a different depth than the afferent one.

    std::vector<u_Data> Values;

    c_Homeostasis_IO_Module()
    {
        Afferent_Count = 0;
        Efferent_Count = 0;

        //Afferent = new c_AE_IO_Element * [1]; Afferent[0] = new c_AE_IO_Element;
        //Efferent = new c_AE_IO_Element * [1]; Efferent[0] = new c_AE_IO_Element;

        Afferent = NULL;
        Efferent = NULL;

        //For afferent and efferent 1 each. The depth of this is based on (Afferent_Count + Efferent_Count)
        Gathered_Afferent[0] = new uint64_t[1]; Gathered_Afferent[0][0] = 0;
        Gathered_Afferent[1] = new uint64_t[1]; Gathered_Afferent[1][0] = 0;
        Gathered_Afferent[2] = new uint64_t[1]; Gathered_Afferent[2][0] = 0;
        Gathered_Efferent = new uint64_t[1]; Gathered_Efferent[0] = 0;

        std::cout << "\n A: " << Afferent_Count << " E: " << Efferent_Count;
    }

    //Register Afferent input. The module always starts with 1.
    int register_Afferent()
    {
        //std::cout << "\n register_Afferent(): ";

        //Declare the tmp arrays.
        c_AE_IO_Element** tmp_A;
        tmp_A = new c_AE_IO_Element * [Afferent_Count];

        //Swap the indices to the tmp array.
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            tmp_A[cou_A] = Afferent[cou_A]; Afferent[cou_A] = NULL;
        }

        //Delete and redeclare the real arrays.
        delete[] Afferent; Afferent = NULL;

        //Redeclare the OG arrays.
        Afferent = new c_AE_IO_Element * [Afferent_Count + 1];

        //create the new indices.
        Afferent[Afferent_Count] = new c_AE_IO_Element;

        //Recover the old data.
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            Afferent[cou_A] = tmp_A[cou_A]; tmp_A[cou_A] = NULL;
        }

        //Cleanup tmp vars
        //Delete and redeclare the real arrays.
        delete[] tmp_A; tmp_A = NULL;

        Afferent_Count++;

        std::cout << "\n register_Afferent(): " << (Afferent_Count - 1);

        return (Afferent_Count - 1);
    }

    //Register Efferent input/output.
    int register_Efferent()
    {
        //Declare the tmp arrays.
        c_AE_IO_Element** tmp_E;
        tmp_E = new c_AE_IO_Element * [Efferent_Count];

        //Swap the indices to the tmp array.
        for (int cou_E = 0; cou_E < Efferent_Count; cou_E++)
        {
            tmp_E[cou_E] = Efferent[cou_E]; Efferent[cou_E] = NULL;
        }

        //Delete and redeclare the real arrays.
        delete[] Efferent; Efferent = NULL;

        //Redeclare the OG arrays.
        Efferent = new c_AE_IO_Element * [Efferent_Count + 1];

        //NULL the new indices.
        Efferent[Efferent_Count] = new c_AE_IO_Element;
		Efferent[Efferent_Count]->set_Type(1);

        //Recover the old data.
        for (int cou_E = 0; cou_E < Efferent_Count; cou_E++)
        {
            Efferent[cou_E] = tmp_E[cou_E]; tmp_E[cou_E] = NULL;
        }

        //Cleanup tmp vars
        //Delete and redeclare the real arrays.
        delete[] tmp_E; tmp_E = NULL;

        Efferent_Count++;

        std::cout << "\n register_Efferent(): " << (Efferent_Count - 1);

        return (Efferent_Count - 1);
    }
	
	void set_Afferent_Goal(int p_Index, double p_Goal)
	{
		Afferent[p_Index]->set_Goal(p_Goal);
	}
	
	void wipe_Afferent_Bands(int p_Index)
	{
		Afferent[p_Index]->wipe_Bands();
	}
	
    //Accepts an input and updates the correct index with it
    void add_Afferent_Granulation(double p_Bottom, double p_Top, int p_Index = -1) //p_Index comes after since it may be 0
    {
        if (p_Index == -1)
        {
            p_Index = Afferent_Count - 1;
        }

        Afferent[p_Index]->add_Granulation(p_Bottom, p_Top);
    }

    //Exposes the set_Depth() so the user can access it through a function.
    //This extends the input depth of each index so that you can store short term memory of sorts.
    void set_Depth(int p_Depth)
    {
        //For every Afferent and Efferent we set the depth.
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            Afferent[cou_A]->set_Depth(p_Depth);
        }
        for (int cou_E = 0; cou_E < Efferent_Count; cou_E++)
        {
            Efferent[cou_E]->set_Depth(p_Depth);
        }
    }

    //Exposes the set_Value() so the user can access it through a function.
    //Assumes p_Index has been registered in the Afferent array already.
    void set_Afferent_Value(int p_Index, double p_Value)
    {
        Afferent[p_Index]->set_Value(p_Value);
    }

    //Exposes the set_Value() so the user can access it through a function.
    //Assumes p_Index has been registered in the Afferent array already.
    void set_Efferent_Value(int p_Index, double p_Value)
    {
        Efferent[p_Index]->set_Value(p_Value);
    }

    //Exposes the set_Value() so the user can access it through a function.
    void shift_Data()
    {
        //For every Afferent and Efferent we shift the data.
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            Afferent[cou_A]->shift_Data();
        }
        for (int cou_E = 0; cou_E < Efferent_Count; cou_E++)
        {
            Efferent[cou_E]->shift_Data();
        }
    }

    //This is done whenever an afferent or efferent is registered.
    void resize_Gathered_Input()
    {
        delete[] Gathered_Afferent[0]; Gathered_Afferent[0] = NULL;
        delete[] Gathered_Afferent[1]; Gathered_Afferent[1] = NULL;
        delete[] Gathered_Afferent[2]; Gathered_Afferent[2] = NULL;
        delete[] Gathered_Efferent; Gathered_Efferent = NULL;

        Gathered_Afferent[0] = new uint64_t[Afferent_Count];
        Gathered_Afferent[1] = new uint64_t[Afferent_Count];
        Gathered_Afferent[2] = new uint64_t[Afferent_Count];
        Gathered_Efferent = new uint64_t[Efferent_Count];

        for (int cou_Index = 0; cou_Index < Afferent_Count; cou_Index++)
        {
            Gathered_Afferent[0][cou_Index] = 0;
            Gathered_Afferent[1][cou_Index] = 0;
            Gathered_Afferent[2][cou_Index] = 0;
        }
        for (int cou_Index = 0; cou_Index < Efferent_Count; cou_Index++)
        {
            Gathered_Efferent[cou_Index] = 0;
        }
    }

    //Gathers the input set.
    void gather_Input()
    {
        //---std::cout << "\n\n gather_Input()";
        for (int cou_Index = 0; cou_Index < Afferent_Count; cou_Index++)
        {
            Gathered_Afferent[0][cou_Index] = Afferent[cou_Index]->get_Value_Data_uint64_t();
            Gathered_Afferent[1][cou_Index] = Afferent[cou_Index]->get_Value_Granulated_uint64_t();
            Gathered_Afferent[2][cou_Index] = Afferent[cou_Index]->get_Value_Delta_uint64_t();

            //---std::cout << "\n   a-[" << cou_Index << "]: C: " << Gathered_Afferent[0][cou_Index] << " G: " << Gathered_Afferent[1][cou_Index] << " D: " << Gathered_Afferent[2][cou_Index];
        }
        //---std::cout << "\n";
        for (int cou_Index = 0; cou_Index < Efferent_Count; cou_Index++)
        {
            Gathered_Efferent[cou_Index] = Efferent[cou_Index]->get_Value_Data_uint64_t();

            //---std::cout << "   e-[" << cou_Index << "]: V: " << Gathered_Efferent[cou_Index];
        }
    }

    //Exposes the output_IO() so the user can access it through a function.
    void output_AE()
    {
        //For every Afferent and Efferent we shift the data.
        std::cout << "\n ---Afferent_Count: " << Afferent_Count;
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            std::cout << "\n    [" << cou_A << "]";
            Afferent[cou_A]->output();
        }
        std::cout << "\n ---Efferent_Count: " << Efferent_Count;
        for (int cou_E = 0; cou_E < Efferent_Count; cou_E++)
        {
            std::cout << "\n    [" << cou_E << "]";
            Efferent[cou_E]->output();
        }
    }

    //Outputs the deviation mapping.
    void output_Deviation_Mapping()
    {
        std::cout << "\n ---Afferent_Count: " << Afferent_Count;
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            std::cout << "\n    [" << cou_A << "]";
            
            for (int cou_Chron = 0; cou_Chron < Afferent[cou_A]->Depth; cou_Chron++)
            {
                std::cout << "(" << Afferent[cou_A]->Deviation[cou_Chron] << ")";
            }
        }
    }

    //Outputs the deviation mapping.
    std::vector<double> get_Current_Deviation_Set()
    {
        std::vector<double> tmp_Return;
        tmp_Return.resize(Afferent_Count);

        //---std::cout << "\n ---Afferent_Count: " << Afferent_Count;
        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            tmp_Return[cou_A] = Afferent[cou_A]->Deviation[Afferent[cou_A]->Depth - 1];
        }

        return tmp_Return;
    }

    std::vector<std::vector<double>> get_Deviation_Set()
    {
        std::vector<std::vector<double>> tmp_Return;
        tmp_Return.resize(Afferent_Count);

        for (int cou_A = 0; cou_A < Afferent_Count; cou_A++)
        {
            tmp_Return[cou_A].resize(Afferent[cou_A]->Depth);

            for (int cou_D = 0; cou_D < Afferent[cou_A]->Depth; cou_D++)
            {
                tmp_Return[cou_A][cou_D] = Afferent[cou_A]->Deviation[cou_D];
            }
        }
        return tmp_Return;
    }

    double get_Current_Afferent_Deviation(int p_Afferent)
    {
        return Afferent[p_Afferent]->Deviation[Afferent[p_Afferent]->Depth - 1];
    }

    //Outputs the gathered Inputs
    void output_Gathered()
    {
        for (int cou_Index = 0; cou_Index < Afferent_Count; cou_Index++)
        {
            std::cout << "\n [0] " << Gathered_Afferent[0][cou_Index];
            std::cout << " [1] " << Gathered_Afferent[1][cou_Index];
            std::cout << " [2] " << Gathered_Afferent[2][cou_Index];
        }
        std::cout << "\n";
        for (int cou_Index = 0; cou_Index < Efferent_Count; cou_Index++)
        {
            std::cout << " {[" << cou_Index << "] " << Gathered_Efferent[cou_Index] << " } ";
        }
    }
	
	void write_State(std::ofstream * p_File)
	{
		if (!p_File->is_open())
		{
			std::cout << "\n ERROR: The c_Homeostasis_IO_Module write_State function was passed an unoped handle...";
		}
		
		//The gathered afferent/efferent
		*p_File << "\nCurrent_Gathered_Afferent_Input";
        for (int cou_Index = 0; cou_Index < Afferent_Count; cou_Index++)
        {
			*p_File << " " << cou_Index;
            *p_File << " 0 " << Gathered_Afferent[0][cou_Index];
            *p_File << " 1 " << Gathered_Afferent[1][cou_Index];
            *p_File << " 2 " << Gathered_Afferent[2][cou_Index];
        }
        *p_File << "\nCurrent_Gathered_Efferent_Input";
        for (int cou_Index = 0; cou_Index < Efferent_Count; cou_Index++)
        {
            *p_File << " [" << cou_Index << "][0] " << Gathered_Efferent[cou_Index];
		}
		
		
        for (int cou_Index = 0; cou_Index < Afferent_Count; cou_Index++)
        {
			*p_File << "\n\nAfferent_Index [" << cou_Index << "]";
			
			*p_File << "\nget_Value_Granulated";
		
			for (int cou_L=0;cou_L<Afferent[cou_Index]->Depth;cou_L++)
			{
				*p_File << " [" << cou_L << "] " << Afferent[cou_Index]->get_Value_Granulated(cou_L);
			}
		
			*p_File << "\nget_Value_Deltat";
		
			for (int cou_L=0;cou_L<Afferent[cou_Index]->Depth;cou_L++)
			{
				*p_File << " [" << cou_L << "] " << Afferent[cou_Index]->get_Value_Delta(cou_L);
			}
		
			*p_File << "\nget_Value_Data";
		
			for (int cou_L=0;cou_L<Afferent[cou_Index]->Depth;cou_L++)
			{
				*p_File << " [" << cou_L << "] " << Afferent[cou_Index]->get_Value_Data(cou_L);
			}
			
			*p_File << "\nCurrent_Deviation_Set";
		
			for (int cou_L=0;cou_L<Afferent[cou_Index]->Depth;cou_L++)
			{
				*p_File << " [" << cou_L << "] " << Afferent[cou_Index]->Deviation[cou_L];
			}
		}
	}
};





























//ChatGPT generated struct
//ChatGPT wrote this, I'm not going to bother fixing the internals. It can stay ugly.
struct TraceSelectionConfig
{
    // How much each component contributes to the composite score [0]
    float weight_SA   = 1.0f;   // Start Anchor
    float weight_DM   = 1.0f;   // Direction Match
    float weight_RC   = 1.0f;   // RC
    float weight_Chrg = 1.0f;   // Charge

    // How you normalize the raw sums into "score space"
    float norm_SA_scale   = 100.0f;  // originally *100
    float norm_DM_scale   = 100.0f;  // originally *100
    float norm_RC_scale   = 10.0f;   // originally *10
    float norm_Chrg_scale = 10.0f;   // originally *10

    // Minimal criteria for “valid trace”
    int   min_start_anchor_percent = 0;   // min normalized SA to consider
    int   min_direction_percent    = 0;   // min normalized DM to consider

    // How many valid traces before declaring “No_Streak” territory
    int   valid_traces_for_no_streak = 250;

    // Global tuning of efferent thresholding
    float score_threshold_modifier = 1.0f;   // multiplies max Fn

    // Optional boredom behaviour tuning
    int   bored_node_delta          = 0;     // allowed delta before flg_Bored
};


//ChatGPT wrote this, I'm not going to bother fixing the internals. It can stay ugly.
TraceSelectionConfig load_TraceSelectionConfig(const std::string& p_FName = "Trace_Selection_Config.ssv")
{
    TraceSelectionConfig cfg; // starts with defaults

    std::ifstream tmp_File(p_FName);
    if (!tmp_File.is_open())
    {
        std::cerr << "\n[TraceConfig] Could not open config file: " << p_FName
                  << " — using defaults.\n";
        return cfg;
    }
    std::string tmp_String = "";
	
	while(!(tmp_File.eof()))
	{
		tmp_File >> tmp_String;
		//std::cout << "\n[LOADED]: " << tmp_String;
		if (tmp_String == ""){ continue; }

		if (tmp_String == "weight_SA"){
			tmp_File >> cfg.weight_SA; std::cout << "\nweight_SA: " << cfg.weight_SA;
		}

		if (tmp_String == "weight_DM"){
			tmp_File >> cfg.weight_DM; std::cout << "\nweight_DM: " << cfg.weight_DM;
		}

		if (tmp_String == "weight_RC"){
			tmp_File >> cfg.weight_RC; std::cout << "\nweight_RC: " << cfg.weight_RC;
		}

		if (tmp_String == "weight_Chrg"){
			tmp_File >> cfg.weight_Chrg; std::cout << "\nweight_Chrg: " << cfg.weight_Chrg;
		}

		if (tmp_String == "norm_SA_scale"){
			tmp_File >> cfg.norm_SA_scale; std::cout << "\nnorm_SA_scale: " << cfg.norm_SA_scale;
		}

		if (tmp_String == "norm_DM_scale"){
			tmp_File >> cfg.norm_DM_scale; std::cout << "\nnorm_DM_scale: " << cfg.norm_DM_scale;
		}

		if (tmp_String == "norm_RC_scale"){
			tmp_File >> cfg.norm_RC_scale; std::cout << "\nnorm_RC_scale: " << cfg.norm_RC_scale;
		}

		if (tmp_String == "norm_Chrg_scale"){
			tmp_File >> cfg.norm_Chrg_scale; std::cout << "\nnorm_Chrg_scale: " << cfg.norm_Chrg_scale;
		}

		if (tmp_String == "min_start_anchor_percent"){
			tmp_File >> cfg.min_start_anchor_percent; std::cout << "\nmin_start_anchor_percent: " << cfg.min_start_anchor_percent;
		}

		if (tmp_String == "min_direction_percent"){
			tmp_File >> cfg.min_direction_percent; std::cout << "\nmin_direction_percent: " << cfg.min_direction_percent;
		}

		if (tmp_String == "valid_traces_for_no_streak"){
			tmp_File >> cfg.valid_traces_for_no_streak; std::cout << "\nvalid_traces_for_no_streak: " << cfg.valid_traces_for_no_streak;
		}

		if (tmp_String == "score_threshold_modifier"){
			tmp_File >> cfg.score_threshold_modifier; std::cout << "\nscore_threshold_modifier: " << cfg.score_threshold_modifier;
		}

		if (tmp_String == "bored_node_delta"){
			tmp_File >> cfg.bored_node_delta; std::cout << "\nbored_node_delta: " << cfg.bored_node_delta;
		}
		tmp_String = "";
	}
	
	tmp_File.close();
	
		/*

    while (std::getline(in, line))
    {
        // Trim leading spaces
        std::size_t pos = line.find_first_not_of(" \t\r\n");
        if (pos == std::string::npos) continue;           // blank line
        if (line[pos] == '#') continue;                   // comment line

        std::istringstream iss(line);
        std::string key;
        float value_f = 0.0f;
        int   value_i = 0;

        if (!(iss >> key))
            continue;

        // Float keys
        if (key == "weight_SA")                { if (iss >> value_f) cfg.weight_SA = value_f; }
        else if (key == "weight_DM")           { if (iss >> value_f) cfg.weight_DM = value_f; }
        else if (key == "weight_RC")           { if (iss >> value_f) cfg.weight_RC = value_f; }
        else if (key == "weight_Chrg")         { if (iss >> value_f) cfg.weight_Chrg = value_f; }
        else if (key == "norm_SA_scale")       { if (iss >> value_f) cfg.norm_SA_scale = value_f; }
        else if (key == "norm_DM_scale")       { if (iss >> value_f) cfg.norm_DM_scale = value_f; }
        else if (key == "norm_RC_scale")       { if (iss >> value_f) cfg.norm_RC_scale = value_f; }
        else if (key == "norm_Chrg_scale")     { if (iss >> value_f) cfg.norm_Chrg_scale = value_f; }
        else if (key == "score_threshold_modifier")
        {
            if (iss >> value_f) cfg.score_threshold_modifier = value_f;
        }
        else if (key == "bored_node_delta")
        {
            if (iss >> value_i) cfg.bored_node_delta = value_i;
        }
        // Int keys
        else if (key == "min_start_anchor_percent")
        {
            if (iss >> value_i) cfg.min_start_anchor_percent = value_i;
        }
        else if (key == "min_direction_percent")
        {
            if (iss >> value_i) cfg.min_direction_percent = value_i;
        }
        else if (key == "valid_traces_for_no_streak")
        {
            if (iss >> value_i) cfg.valid_traces_for_no_streak = value_i;
        }
        else
        {
            // Unknown key; you can log it or ignore quietly.
            std::cerr << "\n[TraceConfig] Unknown key: " << key;
        }
    }
	*/
    return cfg;
}

class c_Projection
{
public:

    std::vector<std::vector<std::vector<u_Data>>> Data;
};


//Each input in the homeostasis module input array is given a granulator that can be configured individually.
class c_Homeostasis_Module
{
public:
	
	TraceSelectionConfig Trace_Selection_Config;
	
    c_Homeostasis_IO_Module IO;

    std::vector<c_Homeostasis_IO_Module> IO_Hist;

    c_Projection Current_Projection;

    std::vector<c_Projection> Projection_History;

    NT4::c_Time_Series_Generator_Module TSG;

    std::vector<int> Output_Signals;
    std::vector<int> No_Streak_Output_Signals;

    int No_Streak;
    int No_Streak_On;
    int No_Streak_Off;

    //Boredom mechanic.
    int Previous_Node_Count;

    int Tick_Count;

    c_Homeostasis_Module()
    {
        No_Streak = 0;
        No_Streak_On = 0;
        No_Streak_Off = 0;
        Previous_Node_Count = 0;
        Tick_Count = 0;
		
		load_Trace_Selection();
    }

    void init(int p_Chrono_Depth)
    {
        IO.set_Depth(p_Chrono_Depth);
        init_TSG(p_Chrono_Depth);
    }

    void load_Trace_Selection()
	{
		Trace_Selection_Config = load_TraceSelectionConfig();
	}
	
	//Call after you've registered all your afferent and efferent.
    void init_TSG(int p_Chrono_Depth)
    {
        //int tmp_IO_Depth = ((IO.Afferent_Count) * 3) + ((IO.Efferent_Count) * 3);
        int tmp_IO_Depth = ((IO.Afferent_Count) * 3) + (IO.Efferent_Count);

        TSG.init(p_Chrono_Depth, tmp_IO_Depth, 5);
    }

    //Encodes an input set.
    void encode(int p_RF = 0)
    {
        TSG.encode(p_RF);
    }

    void write_System_State_Snapshot(std::string p_FName = "./System_State_Files/snapshot.ssv")
	{
		write_AE_State(p_FName);
	}
	
	void write_AE_State(std::string p_FName = "./System_State_Files/snapshot.ssv")
	{
		std::ofstream tmp_File(p_FName);
		
		if (!tmp_File.is_open())
		{
			std::cout << "\n ERROR: Cannot open file [" << p_FName << "] for writing AE state.";
			return;
		}
		
		IO.write_State(&tmp_File);
		
		tmp_File.close();
	}
	
	void write_Bulk(std::string p_FName, int p_Tick)
    {
        std::vector<std::vector<std::vector<NT4::s_Out>>> tmp_Bulk;

        tmp_Bulk = TSG.get_Bulk(1);


        std::ofstream BSF_D;
        std::ofstream BSF_M;
        std::ofstream BSF_C;
        std::ofstream BSF_R;
		/*
        std::string tmp_BName_D = p_FName + "." + std::to_string(p_Tick) + ".Primitive.ssv";
        std::string tmp_BName_M = p_FName + "." + std::to_string(p_Tick) + ".Match.ssv";
        std::string tmp_BName_C = p_FName + "." + std::to_string(p_Tick) + ".Charge.ssv";
        std::string tmp_BName_R = p_FName + "." + std::to_string(p_Tick) + ".RC.ssv";
		*/
		
        std::string tmp_BName_D = p_FName + "Primitive.ssv";
        std::string tmp_BName_M = p_FName + "Match.ssv";
        std::string tmp_BName_C = p_FName + "Charge.ssv";
        std::string tmp_BName_R = p_FName + "RC.ssv";
		
		/*
        BSF_D.open(tmp_BName_D, std::ios::app);
        BSF_M.open(tmp_BName_M, std::ios::app);
        BSF_C.open(tmp_BName_C, std::ios::app);
        BSF_R.open(tmp_BName_R, std::ios::app);
		*/
		
        BSF_D.open(tmp_BName_D, std::ios::app);
        BSF_M.open(tmp_BName_M, std::ios::app);
        BSF_C.open(tmp_BName_C, std::ios::app);
        BSF_R.open(tmp_BName_R, std::ios::app);

        if (tmp_Bulk.size() == 0) { BSF_D << "0"; BSF_M << "0"; BSF_C << "0"; BSF_R << "0"; BSF_D.close();  BSF_M.close();  BSF_C.close();  BSF_R.close(); return; }
        if (tmp_Bulk[0].size() == 0) { BSF_D << "0"; BSF_M << "0"; BSF_C << "0"; BSF_R << "0"; BSF_D.close();  BSF_M.close();  BSF_C.close();  BSF_R.close(); return; }

        for (int cou_O = 0; cou_O < tmp_Bulk[0][0].size(); cou_O++)
        {
            if (cou_O > 0)
            {
                BSF_D << "\n\n\n\n";
                BSF_M << "\n\n\n\n";
                BSF_C << "\n\n\n\n";
                BSF_R << "\n\n\n\n";
            }
            BSF_D << cou_O << " ";
            BSF_M << cou_O << " ";
            BSF_C << cou_O << " ";
            BSF_R << cou_O << " ";

            for (int cou_Raw = 0; cou_Raw < tmp_Bulk[0].size(); cou_Raw++)
            {
                for (int cou_Chrono = 0; cou_Chrono < tmp_Bulk.size(); cou_Chrono++)
                {
                    BSF_D << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D << " ";
                    BSF_M << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Match.D << " ";
                    BSF_C << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Charge << " ";
                    BSF_R << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].RC << " ";
                }
            }
        }
        BSF_D.close();
        BSF_M.close();
        BSF_C.close();
        BSF_R.close();
    }

    void evaluate_Traces(std::string p_FName, float p_Score_Threshold_Modifier)
    {
		load_Trace_Selection();
		
        std::cout << "\n Trace Selection in progress...";
        std::vector<std::vector<std::vector<NT4::s_Out>>> tmp_Bulk;

        tmp_Bulk = TSG.get_Bulk(1);
        write_Bulk("System_State_Files/bulk.", Tick_Count);

        std::vector<double> tmp_Deviation_Mapping;
        tmp_Deviation_Mapping = get_Current_Deviation_Set();
		
		std::ofstream tmp_Stat_File("System_State_Files/trace_eval_stats.ssv", std::ios::trunc);
		if (!tmp_Stat_File.is_open())
		{
			std::cout << "\n ERROR: Could not open System_State_Files/trace_eval_state.ssv for trace stat logging.";
		}

        // [-----]
        // [--+-+]
        // [+++++]
        //I need one of these evaluators for every trace.

        /*
        std::cout << "\n\n\n tmp_Bulk:";
        for (int cou_Chrono = 0; cou_Chrono < tmp_Bulk.size(); cou_Chrono++)
        {
            std::cout << "\n    Chrono[" << cou_Chrono << "]";

            for (int cou_Raw = ((IO.Afferent_Count - 1) * 3); cou_Raw < tmp_Bulk[cou_Chrono].size(); cou_Raw++)
            //for (int cou_Raw = 0; cou_Raw < tmp_Bulk[cou_Chrono].size(); cou_Raw++)
            {
                std::cout << "\n       Raw[" << cou_Raw << "]";
                for (int cou_O = 0; cou_O < tmp_Bulk[cou_Chrono][cou_Raw].size(); cou_O++)
                {
                    std::cout << "\n          O[" << cou_O << "]";
                    std::cout << " Bulk_Primitive: " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
                    std::cout << " Match: " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Match.D;
                    std::cout << " Charge: " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Charge;
                }
            }
        }*/


        //---std::cout << "\n\n _~_ tmp_Bulk Checker:";

        float tmp_Count = 0;

        int tmp_Raw_Depth = 0;
        int tmp_Chrono_Depth = 0;
        int tmp_Output_Depth = 0;

        std::vector<std::vector<int>> tmp_Validate_Start_Anchor;
        std::vector<std::vector<std::vector<int>>> tmp_Validate_Direction;
        std::vector<std::vector<std::vector<int>>> tmp_Validate_VDirection;
        std::vector<std::vector<std::vector<float>>> tmp_Validate_Signals;
        std::vector<std::vector<float>> tmp_Validate_Signals_Sum;
        std::vector<int> tmp_Validate_Start_Anchor_Sum;
        std::vector<int> tmp_Validate_Direction_Sum;
        std::vector<std::vector<float>> tmp_Validate_RC;
        std::vector<float> tmp_Validate_RC_Sum;
        std::vector<float> tmp_Validate_Charge;

        float tmp_Normalized_Start_Anchor_Sum = 0;
        float tmp_Normalized_Direction_Sum = 0;
        float tmp_Normalized_RC_Sum = 0;
        float tmp_Normalized_Charge_Sum = 0;
		
		
        std::vector<std::vector<float>> tmp_Output_Signals;

        bool flg_Bored = false;
		
		tmp_Stat_File << "\n Bulk.size " << tmp_Bulk.size();
        if (tmp_Bulk.size() > 0)
        {
            tmp_Chrono_Depth = int(tmp_Bulk.size());
			tmp_Stat_File << "\n Chrono_Depth/Bulk.size " << tmp_Bulk.size();
            if (tmp_Chrono_Depth > 0)
            {
				tmp_Stat_File << "\n Raw_Depth/Bulk[0].size " << tmp_Bulk.size();
                tmp_Raw_Depth = int(tmp_Bulk[0].size());

                if (tmp_Raw_Depth > 0)
                {
					tmp_Stat_File << "\n Output_Depth/int(tmp_Bulk[0][0].size()) " << int(tmp_Bulk[0][0].size());
                    tmp_Output_Depth = int(tmp_Bulk[0][0].size());
                }
            }
        }
		
		std::ofstream tmp_OTrace("./System_State_Files/salience_map.ssv", std::ios::trunc);
		
		tmp_Stat_File << "\nOutput_Traces";
		std::cout << "\nOutput_Traces";
		for (int cou_O=0;cou_O<tmp_Output_Depth;cou_O++)
		{
			tmp_Stat_File << "\n";
			tmp_Stat_File << "\n[" << cou_O << "].Data.Float ";
			tmp_OTrace << "\n" << cou_O << " ";
			std::cout << "\nTrace[" << cou_O << "] ";
						
			std::cout << " Data.Float <";
			for (int cou_Raw=0;cou_Raw<tmp_Raw_Depth;cou_Raw++)
			{
				for (int cou_Chrono=0;cou_Chrono<tmp_Chrono_Depth;cou_Chrono++)
				{
					tmp_Stat_File << " " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
					tmp_OTrace << " " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
					std::cout << " " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
				}
			}
			std::cout << ">";
			tmp_Stat_File << "\n.\n[" << cou_O << "].Charge";
			
			std::cout << " Charge: ";
			
			tmp_Stat_File << " " << tmp_Bulk[0][0][cou_O].Charge;
			tmp_OTrace << " " << tmp_Bulk[0][0][cou_O].Charge;
			std::cout << " " << tmp_Bulk[0][0][cou_O].Charge;
					
			tmp_Stat_File << "\n[" << cou_O << "].RC";
			std::cout << " RC: ";
			float tmp_RC_Avg = 0.0;
			int tmp_RC_Avg_Count = 0;
			for (int cou_Raw=0;cou_Raw<tmp_Raw_Depth;cou_Raw++)
			{
				for (int cou_Chrono=0;cou_Chrono<tmp_Chrono_Depth;cou_Chrono++)
				{
					tmp_RC_Avg += tmp_Bulk[cou_Chrono][cou_Raw][cou_O].RC;
					tmp_RC_Avg_Count++;
				}
			}
			
			if ((tmp_RC_Avg_Count != 0) && (tmp_RC_Avg != 0))
			{
				tmp_Stat_File << " " << (tmp_RC_Avg / tmp_RC_Avg_Count);
				tmp_OTrace << " " << (tmp_RC_Avg / tmp_RC_Avg_Count);
				std::cout << " " << (tmp_RC_Avg / tmp_RC_Avg_Count);
			}
			else 
			{
				tmp_OTrace << " " << 0;
				tmp_Stat_File << " " << 0;
				std::cout << " " << 0;
				
			}
		}

		tmp_OTrace.close();

        tmp_Validate_Direction.resize(tmp_Chrono_Depth);
        tmp_Validate_VDirection.resize(tmp_Chrono_Depth);

        tmp_Validate_Signals.resize(tmp_Chrono_Depth);
        tmp_Validate_Signals_Sum.resize(tmp_Chrono_Depth);

        tmp_Validate_Start_Anchor.resize(tmp_Raw_Depth);
        tmp_Validate_RC.resize(tmp_Raw_Depth);

        tmp_Output_Signals.resize(IO.Efferent_Count);

        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            //[0]Composite [1]SAS [2]DS [3]RC [4]Chrg
            tmp_Output_Signals[cou_E].resize(5);
        }

        for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
        {
            tmp_Validate_Direction[cou_Chrono].resize(tmp_Raw_Depth);
            tmp_Validate_VDirection[cou_Chrono].resize(tmp_Raw_Depth);

            for (int cou_R = 0; cou_R < tmp_Raw_Depth; cou_R++)
            {
                tmp_Validate_Start_Anchor[cou_R].resize(tmp_Output_Depth);
                tmp_Validate_Direction[cou_Chrono][cou_R].resize(tmp_Output_Depth);
                tmp_Validate_VDirection[cou_Chrono][cou_R].resize(tmp_Output_Depth);
                tmp_Validate_RC[cou_R].resize(tmp_Output_Depth);
            }

            tmp_Validate_Signals[cou_Chrono].resize(IO.Efferent_Count);
            tmp_Validate_Signals_Sum[cou_Chrono].resize(IO.Efferent_Count);

            for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
            {
                tmp_Validate_Signals[cou_Chrono][cou_E].resize(tmp_Output_Depth);
            }
        }

        tmp_Validate_Start_Anchor_Sum.resize(tmp_Output_Depth);
        tmp_Validate_Direction_Sum.resize(tmp_Output_Depth);
        tmp_Validate_Charge.resize(tmp_Output_Depth);
        tmp_Validate_RC_Sum.resize(tmp_Output_Depth);

        //TSG.output_Bulk(1);

        //TSG.output_IO(0);
		tmp_Stat_File << "\n\nOutputs_Found ";
		
        if ((tmp_Chrono_Depth > 0) && (tmp_Raw_Depth > 0))
        {
			tmp_Stat_File << " true";
            for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
            {
                tmp_Validate_Charge[cou_O] = tmp_Bulk[0][0][cou_O].Charge;
            }
        }
		else
		{
			tmp_Stat_File << " false";
		}

        //For afferent count find the matches to the starting value
        //---std::cout << "\n Evalitor:";
        tmp_Stat_File << "\n Evalitor:";
        for (int cou_A = 0; cou_A < IO.Afferent_Count; cou_A++)
        {
            //---std::cout << "\n ___ A[" << cou_A << "]";
            tmp_Stat_File << "\n ___ A[" << cou_A << "]";
            for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
            {
                //---std::cout << "\n ___ ___ O[" << cou_O << "]";
                tmp_Stat_File << "\n ___ ___ O[" << cou_O << "]";
                int tmp_AIndex = (cou_A * 3);

                tmp_Bulk[0][(cou_A * 3)][cou_O].flg_Use = 0;

                //---std::cout << " - AIndex[" << tmp_AIndex << "]";
                //---std::cout << "  D: " << tmp_Bulk[0][tmp_AIndex][cou_O].Data.D;
                //---std::cout << "  I: " << TSG.get_Input(0, 0, tmp_AIndex);
                //---std::cout << "  ?= " << (tmp_Bulk[0][tmp_AIndex][cou_O].Data.D == TSG.get_Input(0, 0, tmp_AIndex));
				
                tmp_Stat_File << " - AIndex[" << tmp_AIndex << "]";
                tmp_Stat_File << "  D: " << tmp_Bulk[0][tmp_AIndex][cou_O].Data.D;
                tmp_Stat_File << "  I: " << TSG.get_Input(0, 0, tmp_AIndex);
                tmp_Stat_File << "  ?= " << (tmp_Bulk[0][tmp_AIndex][cou_O].Data.D == TSG.get_Input(0, 0, tmp_AIndex));
                if (tmp_Bulk[0][tmp_AIndex][cou_O].Data.D == TSG.get_Input(0, 0, tmp_AIndex))
                {
                    tmp_Bulk[0][tmp_AIndex][cou_O].flg_Use = 1;
                    tmp_Validate_Start_Anchor[tmp_AIndex][cou_O] = 1;
                    tmp_Validate_Start_Anchor_Sum[cou_O]++;
                }
                tmp_Validate_RC[tmp_AIndex][cou_O] = tmp_Bulk[0][tmp_AIndex][cou_O].RC;
                tmp_Validate_RC_Sum[cou_O] += tmp_Bulk[0][tmp_AIndex][cou_O].RC;

                tmp_AIndex = ((cou_A * 3) + 1);

                tmp_Stat_File << " - AIndex[" << tmp_AIndex << "]";
                tmp_Stat_File << "  D: " << tmp_Bulk[0][tmp_AIndex][cou_O].Data.D;
                tmp_Stat_File << "  I: " << TSG.get_Input(0, 0, tmp_AIndex);
                tmp_Stat_File << "  ?= " << (tmp_Bulk[0][tmp_AIndex][cou_O].Data.D == TSG.get_Input(0, 0, tmp_AIndex));

                //---std::cout << " - AIndex[" << tmp_AIndex << "]";
                //---std::cout << "  D: " << tmp_Bulk[0][tmp_AIndex][cou_O].Data.D;
                //---std::cout << "  I: " << TSG.get_Input(0, 0, tmp_AIndex);
                //---std::cout << "  ?= " << (tmp_Bulk[0][tmp_AIndex][cou_O].Data.D == TSG.get_Input(0, 0, tmp_AIndex));
                double tmp_D = tmp_Bulk[0][tmp_AIndex][cou_O].Data.D;
                double tmp_I = TSG.get_Input(0, 0, tmp_AIndex);
                //float tmp_I_L = (TSG.get_Input(0, 0, tmp_AIndex) - 1);
                //float tmp_I_H = (TSG.get_Input(0, 0, tmp_AIndex) + 1);
                //if ((tmp_D > tmp_I_L) && (tmp_D < tmp_I_H))
                if (tmp_D == tmp_I)
                {
                    tmp_Bulk[0][tmp_AIndex][cou_O].flg_Use = 1;
                    tmp_Validate_Start_Anchor[tmp_AIndex][cou_O] = 1;
                    tmp_Validate_Start_Anchor_Sum[cou_O]++;
                }

                tmp_Validate_RC[tmp_AIndex][cou_O] = tmp_Bulk[0][tmp_AIndex][cou_O].RC;
                tmp_Validate_RC_Sum[cou_O] += tmp_Bulk[0][tmp_AIndex][cou_O].RC;

                for (int cou_Chrono = 1; cou_Chrono < (tmp_Chrono_Depth); cou_Chrono++)
                {

                    tmp_AIndex = ((cou_A * 3) + 2);

                    //---std::cout << " - Chrono[" << cou_Chrono << "]";
                    //---std::cout << "  D: " << tmp_Bulk[cou_Chrono][tmp_AIndex][cou_O].Data.D;
                    //---std::cout << "  M: " << IO.get_Current_Afferent_Deviation(cou_A);
                    //---std::cout << "  ?= " << (tmp_Bulk[cou_Chrono][tmp_AIndex][cou_O].Data.D == IO.get_Current_Afferent_Deviation(cou_A));

                    tmp_Stat_File << " - Chrono[" << cou_Chrono << "]";
                    tmp_Stat_File << "  D: " << tmp_Bulk[cou_Chrono][tmp_AIndex][cou_O].Data.D;
                    tmp_Stat_File << "  M: " << IO.get_Current_Afferent_Deviation(cou_A);
                    tmp_Stat_File << "  ?= " << (tmp_Bulk[cou_Chrono][tmp_AIndex][cou_O].Data.D == IO.get_Current_Afferent_Deviation(cou_A));
                    if (tmp_Bulk[cou_Chrono][tmp_AIndex][cou_O].Data.D == IO.get_Current_Afferent_Deviation(cou_A))
                    {
                        tmp_Validate_Direction[cou_Chrono][cou_A][cou_O]++;
                        tmp_Validate_Direction_Sum[cou_O]++;
                    }
                    tmp_Validate_RC[tmp_AIndex][cou_O] = tmp_Bulk[0][tmp_AIndex][cou_O].RC;
                    tmp_Validate_RC_Sum[cou_O] += tmp_Bulk[0][tmp_AIndex][cou_O].RC;
                }

                int flg_Good_Signal = 0;
                for (int cou_Chrono = (tmp_Chrono_Depth - 1); cou_Chrono >= 0; cou_Chrono--)
                {
                    if (tmp_Validate_Direction[cou_Chrono][cou_A][cou_O] > 0) { flg_Good_Signal = 1; }

                    tmp_Validate_VDirection[cou_Chrono][cou_A][cou_O] += flg_Good_Signal;
                }
            }
        }

        //Loop through each output trace, then loop through each efferent, loop through each index, if the index is >0 then check the VDirection for every afferent and ++ to the signal if it checks out.
        for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
        {
            for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
            {
                //Get the Efferent index for the concrete data.
                //int tmp_EIndex = (IO.Afferent_Count * 3) + (cou_E * 3);
                int tmp_EIndex = (IO.Afferent_Count * 3) + cou_E;

                for (int cou_Chrono = 0; cou_Chrono < (tmp_Chrono_Depth - 1); cou_Chrono++)
                {
                    bool flg_New = 1;

                    //Checks if the signal is new or already on, edge detection.
                    if (cou_Chrono > 0)
                    {
                        flg_New = !(tmp_Bulk[cou_Chrono - 1][tmp_EIndex][cou_O].Data.D > 0);
                    }

                    if ((tmp_Bulk[cou_Chrono][tmp_EIndex][cou_O].Data.D > 0) && flg_New)
                    {
                        float tmp_Valid_Sig = 0;
                        for (int cou_A = 0; cou_A < IO.Afferent_Count; cou_A++)
                        {
                            for (int cou_F = (cou_Chrono + 1); cou_F < tmp_Chrono_Depth; cou_F++)
                            {
                                if (tmp_Validate_VDirection[cou_F][cou_A][cou_O] > 0)
                                {
                                    tmp_Valid_Sig++;
                                }
                                else
                                {
                                    tmp_Valid_Sig = 0;
                                }
                            }
                            /*
                            if (tmp_Validate_VDirection[cou_Chrono + 1][cou_A][cou_O] > 0)
                            {
                                tmp_Validate_Signals[cou_Chrono][cou_E][cou_O] += 1;
                            }
                            */
                        }

                        tmp_Validate_Signals[cou_Chrono][cou_E][cou_O] = tmp_Valid_Sig;
                    }
                }
            }
        }

        tmp_Normalized_Start_Anchor_Sum = 0;
        for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
        {
            if (tmp_Normalized_Direction_Sum < tmp_Validate_Direction_Sum[cou_O]) { tmp_Normalized_Direction_Sum = float(tmp_Validate_Direction_Sum[cou_O]); }
			if (tmp_Normalized_Start_Anchor_Sum < tmp_Validate_Start_Anchor_Sum[cou_O]){ tmp_Normalized_Start_Anchor_Sum = float(tmp_Validate_Start_Anchor_Sum[cou_O]); }

            if (tmp_Normalized_RC_Sum < tmp_Validate_RC_Sum[cou_O]) { tmp_Normalized_RC_Sum = tmp_Validate_RC_Sum[cou_O]; }
            if (tmp_Normalized_Charge_Sum < tmp_Validate_Charge[cou_O]) { tmp_Normalized_Charge_Sum = tmp_Validate_Charge[cou_O]; }
        }

        if (tmp_Normalized_Start_Anchor_Sum > 0)
        {
            for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
            {
                tmp_Validate_Start_Anchor_Sum[cou_O] = int((tmp_Validate_Start_Anchor_Sum[cou_O] / tmp_Normalized_Start_Anchor_Sum) * Trace_Selection_Config.norm_SA_scale);
                tmp_Validate_Direction_Sum[cou_O] = int((tmp_Validate_Direction_Sum[cou_O] / tmp_Normalized_Direction_Sum) * Trace_Selection_Config.norm_DM_scale);
                tmp_Validate_RC_Sum[cou_O] = float(int((tmp_Validate_RC_Sum[cou_O] / tmp_Normalized_RC_Sum) * Trace_Selection_Config.norm_RC_scale));
                tmp_Validate_Charge[cou_O] = float(int((tmp_Validate_Charge[cou_O] / tmp_Normalized_Charge_Sum) * Trace_Selection_Config.norm_Chrg_scale));
            }
        }
        else
        {
            for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
            {
                tmp_Validate_Start_Anchor_Sum[cou_O] = 0;
                tmp_Validate_Direction_Sum[cou_O] = 0;
                tmp_Validate_RC_Sum[cou_O] = 0;
                tmp_Validate_Charge[cou_O] = 0;
            }

            //Select a random trace if there are any and set the scores to 1 to try it and see what happens.
            if (tmp_Output_Depth > 0)
            {
                int tmp_O = rand() % tmp_Output_Depth;

                tmp_Validate_Start_Anchor_Sum[tmp_O] = 1;
                tmp_Validate_Direction_Sum[tmp_O] = 1;
                tmp_Validate_RC_Sum[tmp_O] = 1;
                tmp_Validate_Charge[tmp_O] = 1;
            }
        }


		std::ofstream tmp_OFTrace("./System_State_Files/salience_map.Filtered.ssv", std::ios::trunc);
		
		tmp_Stat_File << "\nOutput_Traces_Filtered";
		std::cout << "\nOutput_Traces_Filtered";
		for (int cou_O=0;cou_O<tmp_Output_Depth;cou_O++)
		{
			tmp_Stat_File << "\n";
			tmp_Stat_File << "\n[" << cou_O << "].Data.Float ";
			tmp_OFTrace << "\n" << cou_O << " ";
			std::cout << "\nTrace[" << cou_O << "] ";
			
			
            if (tmp_Validate_Direction_Sum[cou_O] < Trace_Selection_Config.min_direction_percent) 
			{ 
				std::cout << " Delta_Match: " << tmp_Validate_Direction_Sum[cou_O];
				std::cout << " / " << Trace_Selection_Config.min_direction_percent;
			}
            if (tmp_Validate_Start_Anchor_Sum[cou_O] < Trace_Selection_Config.min_start_anchor_percent) 
			{ 
				std::cout << " Start_Anchor_Match: " << tmp_Validate_Start_Anchor_Sum[cou_O];
				std::cout << " / " << Trace_Selection_Config.min_start_anchor_percent;
			}
			
            if (tmp_Validate_Direction_Sum[cou_O] < Trace_Selection_Config.min_direction_percent) { continue; }
            if (tmp_Validate_Start_Anchor_Sum[cou_O] < Trace_Selection_Config.min_start_anchor_percent) { continue; }
			
			std::cout << " Data.Float <";
			for (int cou_Raw=0;cou_Raw<tmp_Raw_Depth;cou_Raw++)
			{
				for (int cou_Chrono=0;cou_Chrono<tmp_Chrono_Depth;cou_Chrono++)
				{
					tmp_Stat_File << " " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
					tmp_OFTrace << " " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
					std::cout << " " << tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Data.D;
				}
			}
			std::cout << ">";
			tmp_Stat_File << "\n.\n[" << cou_O << "].Charge";
			
			std::cout << " Charge: ";
			
			tmp_Stat_File << " " << tmp_Bulk[0][0][cou_O].Charge;
			tmp_OFTrace << " " << tmp_Bulk[0][0][cou_O].Charge;
			std::cout << " " << tmp_Bulk[0][0][cou_O].Charge;
					
			tmp_Stat_File << "\n[" << cou_O << "].RC";
			std::cout << " RC: ";
			float tmp_RC_Avg = 0.0;
			int tmp_RC_Avg_Count = 0;
			for (int cou_Raw=0;cou_Raw<tmp_Raw_Depth;cou_Raw++)
			{
				for (int cou_Chrono=0;cou_Chrono<tmp_Chrono_Depth;cou_Chrono++)
				{
					tmp_RC_Avg += tmp_Bulk[cou_Chrono][cou_Raw][cou_O].RC;
					tmp_RC_Avg_Count++;
				}
			}
			
			if ((tmp_RC_Avg_Count != 0) && (tmp_RC_Avg != 0))
			{
				tmp_Stat_File << " " << (tmp_RC_Avg / tmp_RC_Avg_Count);
				tmp_OFTrace << " " << (tmp_RC_Avg / tmp_RC_Avg_Count);
				std::cout << " " << (tmp_RC_Avg / tmp_RC_Avg_Count);
			}
			else 
			{
				tmp_OFTrace << " " << 0;
				tmp_Stat_File << " " << 0;
				std::cout << " " << 0;
				
			}
			std::cout << " Delta_Match: " << tmp_Validate_Direction_Sum[cou_O];
			std::cout << " / " << Trace_Selection_Config.min_direction_percent;
			std::cout << " Start_Anchor_Match: " << tmp_Validate_Start_Anchor_Sum[cou_O];
			std::cout << " / " << Trace_Selection_Config.min_start_anchor_percent;
		}

		tmp_OFTrace.close();


        int tmp_Valid_Traces = 0;

        std::cout << "\n\n Trace Selection Scores:";
        for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
        {
            bool tmp_Flg_Valid_Trace = false;

            if (tmp_Validate_Direction_Sum[cou_O] < Trace_Selection_Config.min_direction_percent) { continue; }
            if (tmp_Validate_Start_Anchor_Sum[cou_O] < Trace_Selection_Config.min_start_anchor_percent) { continue; }

            /*---*/std::cout << "\n[" << cou_O << "] ";
            /*---*/std::cout << " Delta_Direction_Match ";
            for (int cou_A = 0; cou_A < IO.Afferent_Count; cou_A++)
            {
                
                
                std::cout << " " << cou_A << "[";
                for (int cou_Chrono = 1; cou_Chrono < (tmp_Chrono_Depth); cou_Chrono++)
                {
                    if (tmp_Validate_Direction[cou_Chrono][cou_A][cou_O] == 1)
                    {
                        std::cout << "+";
                    }
                    else
                    {
                        std::cout << " ";
                    }
                }
                std::cout << "] ";
                
                /*---*/std::cout << " " << cou_A << " [";
                for (int cou_Chrono = 1; cou_Chrono < (tmp_Chrono_Depth); cou_Chrono++)
                {
                    if (tmp_Validate_VDirection[cou_Chrono][cou_A][cou_O] == 1)
                    {
                        /*---*/std::cout << "+";
                    }
                    else
                    {
                        /*---*/std::cout << " ";
                    }
                }
                /*---*/std::cout << "] ";
                
            }

            /*---*/std::cout << " ... Sig ";

            
            for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
            {
                /*
                std::cout << " " << cou_E << "[";
                for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
                {
                    int tmp_Index = (IO.Afferent_Count * 3) + (cou_E * 3);
                    if (tmp_Bulk[cou_Chrono][tmp_Index][cou_O].Data.D == 1)
                    {
                        std::cout << "+";

                        //[0]Composite [1]SAS [2]DS [3]RC [4]Chrg
                        tmp_Output_Signals[cou_E][1] += tmp_Validate_Start_Anchor_Sum[cou_O];
                        tmp_Output_Signals[cou_E][2] += tmp_Validate_Direction_Sum[cou_O];
                        tmp_Output_Signals[cou_E][3] += tmp_Validate_RC_Sum[cou_O];
                        tmp_Output_Signals[cou_E][4] += tmp_Validate_Charge[cou_O];
                    }

                    if (tmp_Bulk[cou_Chrono][tmp_Index][cou_O].Data.D == -1)
                    {
                        std::cout << " ";
                    }

                    if (tmp_Bulk[cou_Chrono][tmp_Index][cou_O].Data.D == 0)
                    {
                        std::cout << " ";
                    }
                }
                std::cout << "]";
                */



                //---std::cout << " " << cou_E << "[";
                for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
                {
                    if (tmp_Validate_Signals[cou_Chrono][cou_E][cou_O] > 0)
                    {
                        //std::cout << "+";
                        //std::cout << char(tmp_Validate_Signals[cou_Chrono][cou_E][cou_O]);

                        //[0]Composite [1]SAS [2]DS [3]RC [4]Chrg
                        tmp_Output_Signals[cou_E][1] += tmp_Validate_Start_Anchor_Sum[cou_O];
                        tmp_Output_Signals[cou_E][2] += tmp_Validate_Direction_Sum[cou_O];
                        tmp_Output_Signals[cou_E][3] += tmp_Validate_RC_Sum[cou_O];
                        tmp_Output_Signals[cou_E][4] += tmp_Validate_Charge[cou_O];

                        tmp_Flg_Valid_Trace = true;
                    }

                    //tmp_Validate_Signals_Sum[cou_Chrono][cou_E] += tmp_Validate_Signals[cou_Chrono][cou_E][cou_O];

                    //---opchr(char(tmp_Validate_Signals[cou_Chrono][cou_E][cou_O]));
                }
                //---std::cout << "]";

                //---std::cout << " " << cou_E << "[";
                for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
                {
                    //---opchr(char(tmp_Validate_Signals_Sum[cou_Chrono][cou_E]));
                }
                //---std::cout << "]";
            }

            if (tmp_Flg_Valid_Trace == true)
            {
                tmp_Valid_Traces++;
            }

            //---std::cout << "\tSA: " << tmp_Validate_Start_Anchor_Sum[cou_O];
            //---std::cout << "\tDS: " << tmp_Validate_Direction_Sum[cou_O];
            //---std::cout << "\tRC: " << tmp_Validate_RC_Sum[cou_O];
            //---std::cout << "\tChrg: " << tmp_Validate_Charge[cou_O];

            tmp_Stat_File << "\tSA: " << tmp_Validate_Start_Anchor_Sum[cou_O];
            tmp_Stat_File << "\tDS: " << tmp_Validate_Direction_Sum[cou_O];
            tmp_Stat_File << "\tRC: " << tmp_Validate_RC_Sum[cou_O];
            tmp_Stat_File << "\tChrg: " << tmp_Validate_Charge[cou_O];
        }

        int tmp_Almost_Valid_Traces = 0;

        //If no traces try again without the limiter of the starting fixation.
        if (tmp_Valid_Traces == 0)
        {
            std::cout << "\n\n Trace Selection Scores - Start Anchor Limit Removed:";

            for (int cou_O = 0; cou_O < tmp_Output_Depth; cou_O++)
            {
                bool tmp_Flg_Valid_Trace = false;

                if (tmp_Validate_Direction_Sum[cou_O] == 0) { continue; }

                //---std::cout << "\n[" << cou_O << "] ";
                //---std::cout << " DMatch ";
                for (int cou_A = 0; cou_A < IO.Afferent_Count; cou_A++)
                {
                    //---std::cout << " " << cou_A << " [";
                    for (int cou_Chrono = 1; cou_Chrono < (tmp_Chrono_Depth); cou_Chrono++)
                    {
                        if (tmp_Validate_VDirection[cou_Chrono][cou_A][cou_O] == 1)
                        {
                            //---std::cout << "+";
                        }
                        else
                        {
                            //---std::cout << " ";
                        }
                    }
                    //---std::cout << "] ";

                }

                //---std::cout << " ... Sig ";


                for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
                {

                    //---std::cout << " " << cou_E << "[";
                    for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
                    {
                        if (tmp_Validate_Signals[cou_Chrono][cou_E][cou_O] > 0)
                        {
                            //std::cout << "+";
                            //std::cout << char(tmp_Validate_Signals[cou_Chrono][cou_E][cou_O]);

                            //[0]Composite [1]SAS [2]DS [3]RC [4]Chrg
                            tmp_Output_Signals[cou_E][1] += tmp_Validate_Start_Anchor_Sum[cou_O];
                            tmp_Output_Signals[cou_E][2] += tmp_Validate_Direction_Sum[cou_O];
                            tmp_Output_Signals[cou_E][3] += tmp_Validate_RC_Sum[cou_O];
                            tmp_Output_Signals[cou_E][4] += tmp_Validate_Charge[cou_O];

                            //tmp_Output_Signals[cou_E][4] *= (tmp_Deviation_Mapping[cou_E] * tmp_Deviation_Mapping[cou_E]);

                            //tmp_Output_Signals[cou_E][0] = (tmp_Deviation_Mapping[cou_E] * tmp_Deviation_Mapping[cou_E]);


                            tmp_Flg_Valid_Trace = true;
                        }

                        //tmp_Validate_Signals_Sum[cou_Chrono][cou_E] += tmp_Validate_Signals[cou_Chrono][cou_E][cou_O];

                        //opchr(char(tmp_Validate_Signals[cou_Chrono][cou_E][cou_O]));
                    }
                    //---std::cout << "]";

                    //---std::cout << " " << cou_E << "[";
                    for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
                    {
                        //---opchr(char(tmp_Validate_Signals_Sum[cou_Chrono][cou_E]));
                    }
                    //---std::cout << "]";
                }


                if (tmp_Flg_Valid_Trace == true)
                {
                    tmp_Almost_Valid_Traces++;
                }

            }

        }

        std::vector<float> tmp_Highest_Output_Signal;
        tmp_Highest_Output_Signal.resize(5);

        std::vector<float> tmp_Lowest_Output_Signal;
        tmp_Lowest_Output_Signal.resize(5);

        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            if (tmp_Highest_Output_Signal[1] < tmp_Output_Signals[cou_E][1]) { tmp_Highest_Output_Signal[1] = tmp_Output_Signals[cou_E][1]; }
            if (tmp_Highest_Output_Signal[2] < tmp_Output_Signals[cou_E][2]) { tmp_Highest_Output_Signal[2] = tmp_Output_Signals[cou_E][2]; }
            if (tmp_Highest_Output_Signal[3] < tmp_Output_Signals[cou_E][3]) { tmp_Highest_Output_Signal[3] = tmp_Output_Signals[cou_E][3]; }
            if (tmp_Highest_Output_Signal[4] < tmp_Output_Signals[cou_E][4]) { tmp_Highest_Output_Signal[4] = tmp_Output_Signals[cou_E][4]; }
        }


        /*
        tmp_Lowest_Output_Signal[0] = tmp_Highest_Output_Signal[0];
        tmp_Lowest_Output_Signal[1] = tmp_Highest_Output_Signal[1];
        tmp_Lowest_Output_Signal[2] = tmp_Highest_Output_Signal[2];
        tmp_Lowest_Output_Signal[3] = tmp_Highest_Output_Signal[3];
        tmp_Lowest_Output_Signal[4] = tmp_Highest_Output_Signal[4];

        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            if (tmp_Lowest_Output_Signal[1] > tmp_Output_Signals[cou_E][1]) { tmp_Lowest_Output_Signal[1] = tmp_Output_Signals[cou_E][1]; }
            if (tmp_Lowest_Output_Signal[2] > tmp_Output_Signals[cou_E][2]) { tmp_Lowest_Output_Signal[2] = tmp_Output_Signals[cou_E][2]; }
            if (tmp_Lowest_Output_Signal[3] > tmp_Output_Signals[cou_E][3]) { tmp_Lowest_Output_Signal[3] = tmp_Output_Signals[cou_E][3]; }
            if (tmp_Lowest_Output_Signal[4] > tmp_Output_Signals[cou_E][4]) { tmp_Lowest_Output_Signal[4] = tmp_Output_Signals[cou_E][4]; }
        }

        tmp_Highest_Output_Signal[0] -= tmp_Lowest_Output_Signal[0];
        tmp_Highest_Output_Signal[1] -= tmp_Lowest_Output_Signal[1];
        tmp_Highest_Output_Signal[2] -= tmp_Lowest_Output_Signal[2];
        tmp_Highest_Output_Signal[3] -= tmp_Lowest_Output_Signal[3];
        tmp_Highest_Output_Signal[4] -= tmp_Lowest_Output_Signal[4];
        */


        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            //tmp_Output_Signals[cou_E][1] = (tmp_Output_Signals[cou_E][1] - tmp_Lowest_Output_Signal[1]) / tmp_Highest_Output_Signal[1];
            //tmp_Output_Signals[cou_E][2] = (tmp_Output_Signals[cou_E][2] - tmp_Lowest_Output_Signal[2]) / tmp_Highest_Output_Signal[2];
            //tmp_Output_Signals[cou_E][3] = (tmp_Output_Signals[cou_E][3] - tmp_Lowest_Output_Signal[3]) / tmp_Highest_Output_Signal[3];
            //tmp_Output_Signals[cou_E][4] = (tmp_Output_Signals[cou_E][4] - tmp_Lowest_Output_Signal[4]) / tmp_Highest_Output_Signal[4];
            //tmp_Output_Signals[cou_E][1] = (tmp_Output_Signals[cou_E][1] / tmp_Highest_Output_Signal[1]) * 10;
            //tmp_Output_Signals[cou_E][2] = (tmp_Output_Signals[cou_E][2] / tmp_Highest_Output_Signal[2]) * 2;
			if (tmp_Highest_Output_Signal[1] > 0)
			{
				tmp_Output_Signals[cou_E][1] = (tmp_Output_Signals[cou_E][1] / tmp_Highest_Output_Signal[1]) * 1;
			}
			if (tmp_Highest_Output_Signal[2] > 0)
			{
				tmp_Output_Signals[cou_E][2] = (tmp_Output_Signals[cou_E][2] / tmp_Highest_Output_Signal[2]) * 1;
			}
			if (tmp_Highest_Output_Signal[3] > 0)
			{
				tmp_Output_Signals[cou_E][3] = (tmp_Output_Signals[cou_E][3] / tmp_Highest_Output_Signal[3]) * 1;
			}
			
			if (tmp_Highest_Output_Signal[4] > 0)
			{
				tmp_Output_Signals[cou_E][4] = (tmp_Output_Signals[cou_E][4] / tmp_Highest_Output_Signal[4]) * 1;
			}
			
			
			tmp_Output_Signals[cou_E][0] += tmp_Output_Signals[cou_E][1] * Trace_Selection_Config.weight_SA;
			tmp_Output_Signals[cou_E][0] += tmp_Output_Signals[cou_E][2] * Trace_Selection_Config.weight_DM;
			tmp_Output_Signals[cou_E][0] += tmp_Output_Signals[cou_E][3] * Trace_Selection_Config.weight_RC;
			tmp_Output_Signals[cou_E][0] += tmp_Output_Signals[cou_E][4] * Trace_Selection_Config.weight_Chrg;
			
			/*
            for (int cou_Rawr = 1; cou_Rawr < 5; cou_Rawr++)
            {
                tmp_Output_Signals[cou_E][0] += tmp_Output_Signals[cou_E][cou_Rawr];
            }
            
            for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
            {
                //tmp_Output_Signals[cou_E][0] += tmp_Validate_Signals_Sum[cou_Chrono][cou_E];
            }*/

            std::cout << "\nOutput_Signal[" << cou_E << "] ";
            std::cout << "Fn: " << int(tmp_Output_Signals[cou_E][0] * 100) << "      ";
            std::cout << "\tSA: " << int(tmp_Output_Signals[cou_E][1] * Trace_Selection_Config.weight_SA);
            std::cout << "\tDM: " << int(tmp_Output_Signals[cou_E][2] * Trace_Selection_Config.weight_DM);
            std::cout << "\tRC: " << int(tmp_Output_Signals[cou_E][3] * Trace_Selection_Config.weight_RC);
            std::cout << "\tChrg: " << int(tmp_Output_Signals[cou_E][4] * Trace_Selection_Config.weight_Chrg);
			
			
            tmp_Stat_File << "\nOutput_Signal[" << cou_E << "] ";
            tmp_Stat_File << "Fn: " << int(tmp_Output_Signals[cou_E][0] * 100);
            tmp_Stat_File << " SA: " << int(tmp_Output_Signals[cou_E][1] * Trace_Selection_Config.weight_SA);
            tmp_Stat_File << " DM: " << int(tmp_Output_Signals[cou_E][2] * Trace_Selection_Config.weight_DM);
            tmp_Stat_File << " RC: " << int(tmp_Output_Signals[cou_E][3] * Trace_Selection_Config.weight_RC);
            tmp_Stat_File << " Chrg: " << int(tmp_Output_Signals[cou_E][4] * Trace_Selection_Config.weight_Chrg);
        }

        std::cout << "\n\n\t Total Traces Found: " << tmp_Output_Depth;
		
		for (int cou_O=0;cou_O<tmp_Output_Depth;cou_O++)
		{
			
		}
		
        std::cout << "\n\t\t Almost Valid Traces Found: " << tmp_Almost_Valid_Traces;
        std::cout << "\n\t\t Valid Traces Found: " << tmp_Valid_Traces;
		
		//Replacing this with chat gippity code to direct the output to ./System_State_Files/x.ssv for the NegentropicNavigator
		/*
        std::ofstream tmp_OF;
        std::string tmp_FName = "./GaiaTesting/" + p_FName + ".Valid_Traces.ssv";
        tmp_OF.open(tmp_FName, std::ios::app);
        tmp_OF << Tick_Count << " " << tmp_Valid_Traces << "\n";
        tmp_OF.close();
        
        std::ofstream tmp_AOF;
        tmp_FName = "./GaiaTesting/" + p_FName + ".Nearly_Valid_Traces.ssv";
        tmp_AOF.open(tmp_FName, std::ios::app);
        tmp_AOF << Tick_Count << " " << tmp_Almost_Valid_Traces << "\n";
        tmp_AOF.close();

        std::ofstream tmp_NOF;
        tmp_FName = "./GaiaTesting/" + p_FName + ".Node_Count.ssv";
        tmp_NOF.open(tmp_FName, std::ios::app);
        tmp_NOF << Tick_Count << " " << TSG.NT4_Core.Base.Nodes.Node_Count << "\n";
        tmp_NOF.close();

        if ((Previous_Node_Count - TSG.NT4_Core.Base.Nodes.Node_Count) == 0)
        {
            flg_Bored = true;
        }

        Previous_Node_Count = int(TSG.NT4_Core.Base.Nodes.Node_Count); 

        std::ofstream tmp_OTOF;
        tmp_FName = "./GaiaTesting/" + p_FName + ".Total_Output_Traces.ssv";
        tmp_OTOF.open(tmp_FName, std::ios::app);
        tmp_OTOF << Tick_Count << " " << tmp_Output_Depth << "\n";
        tmp_OTOF.close();
        */
		        // --- Trace statistics logging to System_State_Files ---

        // Valid traces over time
        {
            std::ofstream tmp_OF("./System_State_Files/trace_valid.ssv", std::ios::app);
            if (tmp_OF.is_open())
            {
                tmp_OF << Tick_Count << " " << tmp_Valid_Traces << "\n";
            }
        } 

        // Nearly valid traces over time
        {
            std::ofstream tmp_AOF("./System_State_Files/trace_nearly_valid.ssv", std::ios::app);
            if (tmp_AOF.is_open())
            {
                tmp_AOF << Tick_Count << " " << tmp_Almost_Valid_Traces << "\n";
            }
        }

        // Node count over time
        {
            std::ofstream tmp_NOF("./System_State_Files/node_count.ssv", std::ios::app);
            if (tmp_NOF.is_open())
            {
                tmp_NOF << Tick_Count << " " << TSG.NT4_Core.Base.Nodes.Node_Count << "\n";
            }
        }
		
		
        // Boredom detection is still computed the same way, we just log separately.
        if ((Previous_Node_Count - TSG.NT4_Core.Base.Nodes.Node_Count) == 0)
        {
            //flg_Bored = true;
        }

        Previous_Node_Count = int(TSG.NT4_Core.Base.Nodes.Node_Count);

        // Total output traces over time
        {
            std::ofstream tmp_OTOF("./System_State_Files/trace_total_output.ssv", std::ios::app);
            if (tmp_OTOF.is_open())
            {
                tmp_OTOF << Tick_Count << " " << tmp_Output_Depth << "\n";
            }
        }

		
		//ChatGPT coded this block.
		// Log per-effector output scores (confidence breakdown) as a snapshot.
        // Each row: efferent_index  Fn  SA  DM  RC  Chrg
        {
            std::ofstream osf("./System_State_Files/output_scores.ssv", std::ios::trunc);
            if (osf.is_open())
            {
                for (int cou_E = 0; cou_E < IO.Efferent_Count; ++cou_E)
                {
                    osf << cou_E; // efferent index
                    // [0] Composite Fn [1] SA [2] DM [3] RC [4] Charge
                    for (int k = 0; k < 5; ++k)
                    {
                        osf << " " << tmp_Output_Signals[cou_E][k];
                    }
                    osf << "\n";
                }
            }
            else
            {
                std::cerr << "\n[log_Output_Scores] Unable to open ./System_State_Files/output_scores.ssv\n";
            }
        }

		
        Tick_Count++;

        float tmp_High_Fn = 0;
        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            if (tmp_High_Fn < tmp_Output_Signals[cou_E][0]) { tmp_High_Fn = tmp_Output_Signals[cou_E][0]; }
        }
        float tmp_Low_Fn = tmp_High_Fn;
        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            if (tmp_Low_Fn > tmp_Output_Signals[cou_E][0]) { tmp_Low_Fn = tmp_Output_Signals[cou_E][0]; }
        }

        tmp_High_Fn -= tmp_Low_Fn;

        if (tmp_High_Fn == 0.0) { tmp_High_Fn = tmp_Low_Fn; tmp_Low_Fn = 0.0f; }

        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            tmp_Output_Signals[cou_E][0] -= tmp_Low_Fn;

            tmp_Output_Signals[cou_E][0] /= tmp_High_Fn;
        }

        tmp_High_Fn = 0;
        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            if (tmp_High_Fn < tmp_Output_Signals[cou_E][0]) { tmp_High_Fn = tmp_Output_Signals[cou_E][0]; }
        }

        Output_Signals.resize(IO.Efferent_Count);

        tmp_High_Fn *= p_Score_Threshold_Modifier;

        if (tmp_High_Fn == 0.0f) { tmp_High_Fn = 0.1f; }

        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            if (tmp_Output_Signals[cou_E][0] > tmp_High_Fn)
            {
                tmp_Output_Signals[cou_E][0] = 1;
                Output_Signals[cou_E] = 1;
            }
            else
            {
                tmp_Output_Signals[cou_E][0] = 0;
                Output_Signals[cou_E] = -1;
            }
        }

        int tmp_No_Streak_ONOFF = -1;
        if (tmp_Valid_Traces < 250)
        {
            if (No_Streak == 0)
            {
                No_Streak_Output_Signals.resize(IO.Efferent_Count);

                for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
                {
                    No_Streak_Output_Signals[cou_E] = -1;
                }

                //Pick one at random to try.
                for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
                {
                    if ((rand() % 2) == 0)
                    {
                        No_Streak_Output_Signals[cou_E] = 1;
                    }
                }

                No_Streak_On = rand() % IO.Efferent_Count;
                No_Streak_Off = rand() % IO.Efferent_Count;
            }


            No_Streak++;

            if (No_Streak >= (get_Chrono_Depth() * 5))
            {
                No_Streak = 0;
            }

            if (No_Streak > (get_Chrono_Depth() * 3))
            {
                tmp_No_Streak_ONOFF = 1;
            }
        }
        else
        {
            No_Streak = 0;
        }

        std::cout << "\n Afferent_Condition:";
        for (int cou_A = 0; cou_A < IO.Afferent_Count; cou_A++)
        {
            std::cout << "\n A[" << cou_A << "]: ";

            if (IO.get_Current_Afferent_Deviation(cou_A) > 0)
            {
                std::cout << " D[[LOW]______]";
            }
            else if (IO.get_Current_Afferent_Deviation(cou_A) < 0)
            {
                std::cout << " D[_____[HIGH]]";
            }
            else
            {
                std::cout << " D[___________]";
            }
        }
        std::cout << "\n";

        /*
        std::cout << "\n Random_Flailing: ";
        if (tmp_No_Streak_ONOFF == 1)
        { 
            std::cout << "[ON]-----"; 
        } 
        else 
        { 
            std::cout << "----[OFF]"; 
        }

        std::cout << " Current: " << No_Streak << " / " << (get_Chrono_Depth() * 5);

        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            std::cout << "\n E[" << cou_E << "]: ";

            if (tmp_No_Streak_ONOFF == 1)
            {
                if (No_Streak_Output_Signals[cou_E] == 1)
                {
                    std::cout << " NS[[ON]-----]";
                }
                else if (No_Streak_Output_Signals[cou_E] == -1)
                {
                    std::cout << " NS[----[OFF]]";
                }
                else
                {
                    std::cout << " NS[---------]";
                }
            }
            else
            {
                std::cout << " NS[---------]";
            }

            if (Output_Signals[cou_E] == 1)
            {
                std::cout << " S[[ON]-----]";
            }
            else if (Output_Signals[cou_E] == -1)
            {
                std::cout << " S[----[OFF]]";
            }
            else
            {
                std::cout << " S[---------]";
            }
        }
        */


        if (tmp_No_Streak_ONOFF == 1)
        {
            //Output_Signals[No_Streak_Off] = -1;
            //Output_Signals[No_Streak_On] = 1;

            for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
            {
                //Output_Signals[cou_E] = No_Streak_Output_Signals[cou_E];
            }
        }

        if (tmp_Valid_Traces <= 0)
        {
            int tmp_RanRan_Flail = rand() % IO.Efferent_Count;

            Output_Signals[tmp_RanRan_Flail] = 1;
        }
		
		/*
        if (flg_Bored)
        {
            if ((rand() % 5) == 0)
            {
                int tmp_RanRan_Flail = rand() % IO.Efferent_Count;

                Output_Signals[tmp_RanRan_Flail] = 1;

                std::cout << "\n Gaia is bored...";
            }
        }
		*/
		
		//Chat GPT generated boredome function when I told it to procedurally iterate through the efferent instead of randomly flailing
		// Boredom-driven procedural efferent testing.
		// When bored, we cycle through efferents in order, and for each one we
		// hold it high for about 1/3 of the Chrono_Depth worth of ticks.
		// During the test we keep all other efferents at 0 so the behaviour is "clean".

		if (flg_Bored && IO.Efferent_Count > 0)
		{
			// Persistent state across calls
			static int s_Bored_Test_Index = 0;   // which efferent we are testing
			static int s_Bored_Test_Ticks = 0;   // how many ticks spent on this efferent

			// Make sure index stays in range (in case efferent count changes)
			if (s_Bored_Test_Index >= IO.Efferent_Count)
			{
				s_Bored_Test_Index = 0;
				s_Bored_Test_Ticks = 0;
			}

			// How long to hold each efferent in the "on" state:
			//  - use 1/3 of Chrono_Depth,
			//  - but never less than 1 tick.
			int testDuration = TSG.Chrono_Depth / 3;
			if (testDuration < 1) { testDuration = 1; }

			// Default baseline: all efferents off.
			for (int e = 0; e < IO.Efferent_Count; ++e)
			{
				Output_Signals[e] = 0;
			}

			// Turn *one* efferent "on": the one we're currently testing.
			Output_Signals[s_Bored_Test_Index] = 1;

			// Optional debug message (comment out if noisy)
			// std::cout << "\nGaia is bored... testing efferent[" << s_Bored_Test_Index
			//           << "] tick " << s_Bored_Test_Ticks << "/" << testDuration;

			// Advance the per-eff test tick counter
			s_Bored_Test_Ticks++;

			// Once we've held this efferent for long enough, move to the next one
			if (s_Bored_Test_Ticks >= testDuration)
			{
				s_Bored_Test_Ticks = 0;
				s_Bored_Test_Index++;

				if (s_Bored_Test_Index >= IO.Efferent_Count)
				{
					s_Bored_Test_Index = 0; // wrap around
				}
			}
		}


		
        // Log boredom flag over time.
        {
            std::ofstream bf("./System_State_Files/boredom.ssv", std::ios::app);
            if (bf.is_open())
            {
                bf << Tick_Count << " " << (flg_Bored ? 1 : 0) << "\n";
            }
        }

        /*
        for (int cou_E = 0; cou_E < IO.Efferent_Count; cou_E++)
        {
            std::cout << "\n[" << cou_E << "] " << tmp_Output_Signals[cou_E][0];

            for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
            {
                std::cout << "\t{" << cou_Chrono << " " << tmp_Validate_Signals_Sum[cou_Chrono][cou_E] << "}";
            }
            std::cout << "]";

        }*/

        /*
        //for (int cou_Raw = (IO.Afferent_Count * 3); cou_Raw < tmp_Raw_Depth; cou_Raw++)
        for (int cou_Raw = 0; cou_Raw < tmp_Raw_Depth; cou_Raw++)
        {
            std::cout << "\n ___  ___ Raw[" << cou_Raw << "]:";
            for (int cou_O = 0; cou_O < tmp_Bulk[0][cou_Raw].size(); cou_O++)
            {
                std::cout << "\n ___  ___  ___ Output[" << cou_O << "]";

                tmp_Count = 0;

                std::cout << " start_Anchor[";
                //if (tmp_Bulk[0][cou_Raw][cou_O].flg_Use == 1)
                if (tmp_Validate_Start_Anchor[cou_Raw][cou_O] == 1)
                {
                    std::cout << "+";
                }
                else
                {
                    std::cout << "-";
                }
                std::cout << "]";

                std::cout << "_" << tmp_Validate_Start_Anchor_Sum[cou_O];

                std::cout << " Match[";
                for (int cou_Chrono = 0; cou_Chrono < tmp_Chrono_Depth; cou_Chrono++)
                {
                    //tmp_Count += tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Match.D;


                    if (tmp_Bulk[cou_Chrono][cou_Raw][cou_O].Match.D == 1)
                    {
                        std::cout << "+";
                    }
                    else
                    {
                        std::cout << "-";
                    }

                }
                std::cout << "]";
                //std::cout << " Match Count: " << tmp_Count;
            }
        }
        */
        std::cout << "\n\n______\n";
        //std::system("PAUSE");
		
		write_System_State_Snapshot();
    }
	


    int get_Output_Signals(int p_Index)
    {
        return Output_Signals[p_Index];
    }

    //Encodes an input set.
    void eval(std::string p_FName, float p_Score_Threshold_Modifier)
    {
        //Generate the current trajectory prediction.
        TSG.eval(0);

        std::vector<std::vector<u_Data>> tmp_Prediction;

        tmp_Prediction = get_Current_Projection(0);

        //p_ProtoGaia->output_AE();

        //output_Deviation_Mapping();

        std::vector<double> tmp_Deviation_Set;

        tmp_Deviation_Set = get_Current_Deviation_Set();

        //copy_Deviation(1);
        copy_Deviation(1);

        //TSG.output_IO(1);

        //Get the deviation search.
        TSG.eval(1);

        evaluate_Traces(p_FName, p_Score_Threshold_Modifier);

        /*
        for (int cou_D = 0; cou_D < tmp_Deviation_Set.size(); cou_D++)
        {
            //copy_Deviation(1);
            std::cout << "\n\nDeviation_Set[" << cou_D << "]";
            copy_Deviation_Index(1, cou_D);

            TSG.output_IO(1);

            //Get the deviation search.
            TSG.eval(1);

            evaluate_Traces();

        }
        */
        //TSG.output_Bulk(1);

    }
	
	void set_Afferent_Goal(int p_Index, double p_Goal)
	{
		IO.set_Afferent_Goal(p_Index, p_Goal);
	}
	
	void wipe_Afferent_Bands(int p_Index)
	{
		IO.wipe_Afferent_Bands(p_Index);
	}
	
    int register_Afferent()
    {
        int tmp_Return = IO.register_Afferent();



        return tmp_Return;
    }

    int register_Efferent()
    {
        return IO.register_Efferent();
    }

    void add_Afferent_Granulation(double p_Bottom, double p_Top, int p_Index = -1)
    {
        return IO.add_Afferent_Granulation(p_Bottom, p_Top, p_Index);
    }

    void set_Depth(int p_Depth)
    {
        IO.set_Depth(p_Depth);
    }

    void set_Chrono_Depth(int p_Chrono_Depth)
    {
        TSG.set_Chrono_Depth(p_Chrono_Depth);
    }

    void update_MSC_Depth() 
    { 
        TSG.update_MSC_Depth(); 
    }

    int get_Chrono_Depth()
    {
        return TSG.Chrono_Depth;
    }

    int get_Afferent_Count()
    {
        return IO.Afferent_Count;
    }

    int get_Efferent_Count()
    {
        return IO.Efferent_Count;
    }

    std::vector<std::vector<u_Data>> get_Current_Projection(int p_RF)
    {
        return TSG.RF[p_RF].Output;
    }

    void output_Current_Projection(int p_RF)
    {
        std::vector<std::vector<u_Data>> tmp_Projection;

        tmp_Projection = get_Current_Projection(p_RF);

        std::cout << "\n Current_Projection[" << p_RF << "]:";
        for (int cou_C = 0; cou_C < tmp_Projection.size(); cou_C++)
        {
            std::cout << "\n";

            for (int cou_I = 0; cou_I < tmp_Projection[cou_C].size(); cou_I++)
            {
                std::cout << " " << tmp_Projection[cou_C][cou_I].D;
            }
        }
    }

    std::vector<u_Data> get_Random_Raw_Projection(int p_Raw)
    {
        return TSG.get_Random_Raw_Projection(p_Raw);
    }

    void shift_Data()
    {
        IO.shift_Data();
        TSG.shift_Data(0);
        TSG.shift_Data(1);
    }

    void set_Afferent_Value(int p_Index, float p_Value, bool p_Con = 1, bool p_Gran = 1, bool p_Delt = 1)
    {
        IO.set_Afferent_Value(p_Index, p_Value);

        //The TSG does straight indexing, so we step by three for con/gran/delta
        int tmp_Index = (p_Index * 3);

        //std::cout << "\n set_Afferent_Value(" << p_Index << ", " << p_Value << "): tmp_Index: " << tmp_Index;
        /*-*/if (p_Con) { TSG.set_Input_Index(0, (tmp_Index + 0), IO.Afferent[p_Index]->get_Value_Data_uint64_t()); }
        //---if (p_Con) { TSG.set_Input_Index(0, (tmp_Index + 0), IO.Afferent[p_Index]->get_Value_Granulated_uint64_t()); }

        //std::cout << "\n set_Afferent_Value(" << p_Index << ", " << p_Value << "): tmp_Index: " << tmp_Index;

        if (p_Gran) { TSG.set_Input_Index(0, (tmp_Index + 1), IO.Afferent[p_Index]->get_Value_Granulated_uint64_t()); }
        //std::cout << "\n set_Afferent_Value(" << p_Index << ", " << p_Value << "): tmp_Index: " << tmp_Index;

        if (p_Delt) { TSG.set_Input_Index(0, (tmp_Index + 2), IO.Afferent[p_Index]->get_Value_Delta_uint64_t()); }

        //std::cout << "\n set_Afferent_Value(" << p_Index << ", " << p_Value << "): tmp_Index: " << tmp_Index;
    }
    
    //The index is passed as the index of the Efferent array, which is separated from the Afferent. Meaning the steps are not in sync.
    //The tmp_Index holds the offset needed to point to the correct index in the straight-indexing of the TSG.
    void set_Efferent_Value(int p_Index, float p_Value)
    {
        IO.set_Efferent_Value(p_Index, p_Value);

        //The TSG does straight indexing, so we step by three for con/gran/delta
        int tmp_Index = ((IO.Afferent_Count) * 3) + p_Index;

        //---std::cout << "\n set_Efferent_Value(" << p_Index << ", " << p_Value << "): tmp_Index: " << tmp_Index;

        TSG.set_Input_Index(0, (tmp_Index + 0), IO.Efferent[p_Index]->get_Value_Data_uint64_t());

        //---std::cout << "\n set_Efferent_Value Complete";
        //---TSG.set_Input_Index(0, (tmp_Index + 1), IO.Efferent[p_Index]->get_Value_Data_uint64_t());
        //---TSG.set_Input_Index(0, (tmp_Index + 1), IO.Efferent[p_Index]->get_Value_Granulated_uint64_t());
        //---TSG.set_Input_Index(0, (tmp_Index + 2), IO.Efferent[p_Index]->get_Value_Delta_uint64_t());
    }

    void output_Deviation_Mapping()
    {
        IO.output_Deviation_Mapping();
	}

    std::vector<double> get_Current_Deviation_Set()
    {
        return IO.get_Current_Deviation_Set();
    }
    std::vector<std::vector<double>> get_Deviation_Set()
    {
        return IO.get_Deviation_Set();
    }

    void copy_Deviation(int p_RF)
    {
        std::vector<double> tmp_Current_Deviation_Set = get_Current_Deviation_Set();

        //---std::cout << "\n copy_Deviation(" << p_RF << ")";

        //The delta are stored in [2]

        for (int cou_Chron = 0; cou_Chron < TSG.Chrono_Depth; cou_Chron++)
        {
            TSG.shift_Data(p_RF, -9999);

            //TSG.set_Input_Index(p_RF, 0, IO.Afferent[0]->get_Value_Data());
            //TSG.set_Input_Index(p_RF, 1, IO.Afferent[0]->get_Value_Granulated());

            for (int cou_A = 1; cou_A < (tmp_Current_Deviation_Set.size() - 1); cou_A++)
            {
                //---std::cout << "\n -~- cou_A " << cou_A;
                //---std::cout << "\n -~- tmp_Current_Deviation_Set[cou_A] " << tmp_Current_Deviation_Set[cou_A];
                int tmp_Index = (cou_A * 3);

                //---std::cout << " -~- tmp_Index " << tmp_Index;

                TSG.set_Input_Index(p_RF, (tmp_Index + 2), tmp_Current_Deviation_Set[cou_A]);
            }
        }
        //---TSG.output_IO(p_RF);
    }

    void copy_Deviation_Index(int p_RF, int p_Index)
    {
        double tmp_Current_Deviation = IO.get_Current_Afferent_Deviation(p_Index);

        //---std::cout << "\n copy_Deviation(" << p_RF << ")";

        //The delta are stored in [2]

        TSG.shift_Data(p_RF, -9999);

        for (int cou_Chron = 1; cou_Chron < (TSG.Chrono_Depth - 1); cou_Chron++)
        {
            TSG.shift_Data(p_RF, -9999);

            //TSG.set_Input_Index(p_RF, 0, IO.Afferent[0]->get_Value_Data());
            //TSG.set_Input_Index(p_RF, 1, IO.Afferent[0]->get_Value_Granulated());

            //---std::cout << "\n -~- cou_A " << cou_A;
            //---std::cout << "\n -~- tmp_Current_Deviation_Set[cou_A] " << tmp_Current_Deviation_Set[cou_A];
            int tmp_Index = (p_Index * 3);

            //---std::cout << " -~- tmp_Index " << tmp_Index;

            TSG.set_Input_Index(p_RF, (tmp_Index + 2), tmp_Current_Deviation);
        }

        TSG.shift_Data(p_RF, -9999);
        //---TSG.output_IO(p_RF);
    }

    void output_TSG()
    {
        TSG.output_Everything();
    }

    void output_NNet()
    {
        TSG.NT4_Core.output_Node_Network();
    }

    void output_Scaffolds()
    {

        TSG.output_Scaffolds();
    }

    void log_Scaffolds(int p_Tick)
    {
		
		std::ofstream tmp_OFile("System_State_Files/scaffolds.ssv", std::ios::app);
		tmp_OFile << "\n" << p_Tick;
		tmp_OFile.close();
        TSG.log_Scaffolds();
    }

    void output_AE()
    {
        IO.output_AE();
    }
};





















/** \addtogroup Construct_Text_Server
 *  @{
 */

 /** \class c_GaiaOS_Text_Server
     \brief This is a handshake based text interface for the engine.


     File-Based Command Control System for Asynchronous Standalone GaiaOS Control:

     Text-Based Command Execution:
     - Commands are represented as text strings that specify actions or operations to be performed by the system or process.
     - Each command is associated with a specific functionality or operation within the system.

     File I/O:
     - Input and output operations are performed using files as the medium of communication.
     - Commands are read from input files, and outputs, returns, and status indicators are written to output files, though some commands do output to the console.
     - This method facilitates asynchronous communication between systems, as they can read from and write to files independently.

     Handshake Protocol:
     - A handshake protocol is used to establish communication or synchronize actions between systems.
     - In this context, a flag file serves as a mechanism for signaling the readiness or completion of certain operations, namely that the user has written a command sequence to 'INPUT/OUTPUT_NAME.ssv' that is ready to be interpreted..
     - The content of the flag file is used to indicate the status of the input/output index, allowing the system to either wait or act accordingly.

     Flag File:
     - A flag file is a small file used to signal the status or state of the system.
     - It typically contains simple data or metadata, such as a single value or indicator, to convey information. In the NT4 text interface it is a boolean value.
     - Flag files are used as synchronization primitives to coordinate activities between systems.

     Control Panel:
     - The control panel represents a set of commands or instructions that can be executed by the system or process.
     - It is stored in a the file "Control_Panel.ssv", which contains a sequence of commands or directives to be processed.

     IO:
     - For each registered Afferent input a file named "Input.n.ssv" is created along with the "Input_Flag.n.ssv" handshake file. These files will be how the user can feed input into GaiaOS.
     - For each registered Efferent output a file named "Output.n.ssv" is created along with the "Output_Flag.n.ssv" handshake file. These files are how GaiaOS outputs the afferent signals.
     - When querying information such as node counts, current symbol mappings, activation mappings, etc the user can specify the filename to output to, and whether to do append or truncate the output.

     Interpretation and Execution:
     - The system interprets commands read from the control file (e.g., Control_Panel.ssv) and executes them sequentially.
     - Command execution may involve performing operations on data, manipulating the state of the system, or interacting with external resources.

     Status Checking and Control Flow:
     - The system periodically checks the status of a flag file (e.g., Control_Panel_Flag.ssv, Input.0.ssv, etc) to determine if there are new commands to execute, input to load, or if certain operations have completed.
     - Based on the status indicated by the flag file, the system may initiate specific actions, continue processing, or wait for further instructions.
     - If an output file then the system flips the handshake to initiated when it is done writing to the output file.

     Startup and 'autoexec.ssv':
     - For 'booting up' the system interprets the commands found in 'autoexec.ssv' sequentially. This allows for shell scripting a startup sequence.

     Custom Commands:
     - A script file placed in "./Scripts/" can be called by simply entering the filename as you would a command token. A file can call other files so you can create dependency hell for yourself if you wish, but with the added complexity of ML.

     \var string const RETURN_FILE defines the file in which returns write their data at this level of the API.
 */

const std::string RETURN_FILE = "./Output/returned.ssv";
class c_GaiaOS_Text_Server
{
    //The construct to hook into.
    c_Homeostasis_Module API;

    //Current server tick.
    int Tick;
	
	//Current Neuro-Semiotic Symbol Processor engine tick.
    int Tick_Processor;

    //Exit flag to allow for exit after startup if the user puts 'exit' into the autoexec file, needed for CLI capabilities.
    bool flg_Exit;
	
	//Whether to load the live update script, can be turned off for poking the machine
	bool flg_Run_Update;

private:


    //    ---==  write_to_output  ==---
    int write_to_output(std::string p_FName, uint64_t p_Data)
    {
        //std::cout << "\n\n __COMMAND__| write_to_output |";
        //std::cout << " - FName \"" << p_FName << "\" Data:" << p_Data;
        std::ofstream tmp_Out(p_FName, std::ios::app);

        // Check if the flag file exists and can be opened
        if (tmp_Out.is_open())
        {
            tmp_Out << p_Data;
        }

        tmp_Out.close();


        return 1;
    }

    //    ---==  write_to_output  ==---
    int write_to_output(std::string p_FName, std::string p_Data)
    {
        //std::cout << "\n\n __COMMAND__| write_to_output |";

        std::ofstream tmp_Out(p_FName, std::ios::app);

        // Check if the flag file exists and can be opened
        if (tmp_Out.is_open())
        {
            tmp_Out << p_Data;
        }

        tmp_Out.close();


        return 1;
    }


    //I am currently writing the commands to register afferent and efferent I/O, plus basic state information output to user defined files with either app or ate by their choice. Once that is done I will then write the portion to handle the actual handshake protocol, assigning each afferent & efferent an index and a file coesponding to the index akin to a PLC. Then onto writing the portions to pipe the commands to the GaiaOS c_Homeostasis_Module. After that it is documentation, cleanup, and hosting on GitHub

    /*
    int eval_Command(std::string p_Command, std::ifstream* p_File)
    {
        //See end of class
    */

    // Load Control_Panel.ssc & issue commands
    int interpret_File(std::string p_LFName)
    {
        std::ifstream LF(p_LFName);

        std::string tmp_In = "";
        int tmp_Count = 0;

        if (LF.is_open())
        {
            while (!LF.eof())
            {
                tmp_In = "";

                LF >> tmp_In;

                if (tmp_In == "") { continue; }

                //std::cout << "\n - [ " << tmp_Count << " ]: " << tmp_In;
                tmp_Count++;


                int tmp_Result = 0;
                tmp_Result = eval_Command(tmp_In, &LF);

                if (tmp_Result == -1) { return -1; }
            }

            return 1;
        }
        else
        {
            std::cerr << "\n Unable to open ScriptFile " << p_LFName << "...\n";

            return 0;
        }


        return 1;
    }


    //Control_Panel_Flag.ssv - Used to signal that there is a live message to read. Set by the user or an external program after inputs and controls are set and ready to have a function executed.
    int check_Control_Panel_Flag()
    {
        std::ifstream flagFile("Control_Panel_Flag.ssv");
        std::string flagValue = "";

        // Check if the flag file exists and can be opened
        if (flagFile.is_open())
        {
            flagFile >> flagValue;

            // Read the value from the flag file
			// Check if the value is 1
			if (flagValue == "1")
			{
				flagFile.close();
				std::ofstream flagFile("Control_Panel_Flag.ssv", std::ios::trunc);
				
				if (flagFile.is_open())
				{
					flagFile << "0";
				}
				else
				{
					std::cerr << "\nUnable to open flag file for clearing.\n";
				}

				return 1;
			}
            else
            {
                flagFile.close();
            }
        }
        else
        {
            std::cerr << "\nUnable to open flag file.\n";
        }

        return 0;
    }

    //Update_Flag.ssv - Used to signal a step in the loop. Set by the user or an external program after inputs and controls are set and ready to have a function executed.
    int check_Update_Flag()
    {
        std::ifstream flagFile("Update_Flag.ssv");
        std::string flagValue = "";

        // Check if the flag file exists and can be opened
        if (flagFile.is_open())
        {
            flagFile >> flagValue;

            // Read the value from the flag file
			// Check if the value is 1
			if (flagValue == "1")
			{
				flagFile.close();
				std::ofstream flagFile("Update_Flag.ssv", std::ios::trunc);
				
				if (flagFile.is_open())
				{
					flagFile << "0";
				}
				else
				{
					std::cerr << "\nUnable to open update flag file for clearing.\n";
				}

				return 1;
			}
            else
            {
                flagFile.close();
            }
        }
        else
        {
            std::cerr << "\nUnable to open update flag file.\n";
        }

        return 0;
    }

    int execute_Control_Panel_Buffer()
    {

        int tmp_Result = 0;
        //tmp_Result = interpret_Control_Panel();
        tmp_Result = interpret_File("Control_Panel.ssv");

        if (tmp_Result)
        {
            std::ofstream clsFinishFlagFile("Control_Panel_Finished.ssv", std::ios::ate);

            // Check if the flag file exists and can be opened
            if (clsFinishFlagFile.is_open())
            {
                clsFinishFlagFile << "1";
            }

            clsFinishFlagFile.close();

        }

        if (tmp_Result == 0)
        {
            std::cerr << "\n\n   ERROR: Unable to interpret the control panel file...  \n";

            return 0;
        }

        if (tmp_Result == -1)
        {
            std::ofstream clsFinishFlagFile("Control_Panel_Finished.ssv", std::ios::ate);

            // Check if the flag file exists and can be opened
            if (clsFinishFlagFile.is_open())
            {
                clsFinishFlagFile << "1";
            }

            clsFinishFlagFile.close();

            return -1;
        }


        std::ofstream file_Object("Control_Panel_Flag.ssv", std::ios::ate);

        //Make sure the file was opened.
        if (!file_Object.is_open())
        {
            std::cerr << "\n\n   ERROR: Unable to interpret the file_Object file for truncation!...  \n";
        }

        file_Object.close();

        return 1;
    }


    int view_File(std::string p_FileName)
    {
        std::ifstream InputFile(p_FileName);

        std::string tmp_In = "";
        int tmp_Count = 0;

        if (InputFile.is_open())
        {
            while (!InputFile.eof())
            {
                InputFile >> tmp_In;
                std::cout << "\n - [ " << tmp_Count << " ]: " << tmp_In;
                tmp_Count++;

                //Neuralman.output_Input();
            }

            std::cout << "\n " << p_FileName << " loaded successfully.\n";

            return 1;
        }
        else
        {
            std::cerr << "\n Unable to open " << p_FileName << " ...\n";

            return 0;
        }
    }

public:

    //The default of the ../ is so that is navigates up from the scripts folder when finding boot status. It's so dumb I find it funny so now this engine's default autoexec file is "./scripts/../autoexec.ssv" lmao. If it doesn't work on other systems I'll have to change it but on windows it works (o.O)
    c_GaiaOS_Text_Server(std::string p_Autoexec = "../autoexec.ssv")
    {
        Tick = 0;
        Tick_Processor = 0;
        flg_Exit = false;
		flg_Run_Update = false;

        std::cout << "\n\n   (~.~) BOOTING UP  ";
        //See if they submitted a command, these scripts are retrieved from the ./Scripts/ dir.
        std::string tmp_Autoexec_FName = "./Scripts/" + p_Autoexec;
        std::cout << "\n\n   (o.o) LOADING BOOT FILE " << tmp_Autoexec_FName << "  \n\n";

        int tmp_Boot_Status = interpret_File(tmp_Autoexec_FName);

        //Load the boot sequence 
        if (tmp_Boot_Status)
        {
            std::cout << "\n\n   \\(^-^)/ SUCCESSFULLLY BOOTED  \n\n";
        }
        else
        {
            std::cout << "\n\n   (;_;) < FAILED TO BOOT PROPERLY  \n\n";
        }

        if (tmp_Boot_Status == -1) { flg_Exit = true; }
    }

    /*
    void save_Config(std::ifstream * p_File)
    {
        std::cout << "\n --> save_Config CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        API.save_Config(tmp_Construct);
    }

    void update_Config(std::ifstream* p_File)
    {
        std::cout << "\n --> update_Config CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        API.update_Config(tmp_Construct);
    }
    */



    //      ---==================================---
    //     ---====================================---
    //    ---======================================---
    //   ---========================================---
    //  ---==   Generic commands for the engine.   ==---
    //   ---========================================---
    //    ---======================================---
    //     ---====================================---
    //      ---==================================---

    //    ---==  clear_output  ==---
    /** Wipes and output file for a given construct, CLS for the output file.

        clear_Output CONSTRUCT_ID
    \param CONSTRUCT_ID The ID of the construct who's output file is getting deleted.
    \retval None This function doesn't return any values.

    Opens the output file associated with the given construct using truncate (ios::ate) to wipe the file.

    Used in scripting for manipulating the output files through the engine to wipe them, or manually, but this allows obtuse control through the engine.

    Example Usage:

    Here we setup a construct, encode a string, gather the treetop node into the output file, output the output, wipe the output using this function, then recheck the output file to make sure:

        register_Construct Many_To_One ExaCon
        set_Input 0 Night Gaunt /end/
        encode 0
        gather_Treetop_Node 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        11 2 11 1 2 3 4 5 6 7 8 9 10 5 0 11 Night Gaunt

    Now we use this function to wipe the file:

        clear_Output 0

    Output:
    Contents of "./Output/ExaCon.Output.ssv":



    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int clear_Output(std::ifstream* p_File)
    {
        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        //Gathers the treetop node.
        //API.clear_Output(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }

    /** You give it a construct ID and it outputs a newline to the output file associated with that construct.

        output_Newline CONSTRUCT_ID

    \param CONSTRUCT_ID The construct to whom the newline shall go, to the coffers of their output file the newline appends.
    \retval None This function doesn't return any values, it outputs \n to the output file.

    Used for formatting output when testing, playing, prototyping, experimenting, or scripting.

    Example Usage:

    For an example we will register an Construct, encode several items, then output them one by one with two newlines betwixt them:

        register_Construct Many_To_One ExaCon
        set_Input 0 Night Gaunt /end/
        encode 0
        gather_Treetop_Node 0

        output_Newline 0
        set_Input 0 Wubbajack /end/
        encode 0
        gather_Treetop_Node 0

        output_Newline 0
        set_Input 0 Ghoul /end/
        encode 0
        gather_Treetop_Node 0

    Output:

    Contents of the file "./Output/ExaCon.Output.ssv":

        11 2 11 1 2 3 4 5 6 7 8 9 10 5 0 11 Night Gaunt
        17 2 9 12 9 13 13 8 14 8 15 16 0 9 Wubbajack
        20 2 5 7 4 18 9 19 0 5 Ghoul

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int output_Newline(std::ifstream* p_File)
    {
        std::cout << "\n --> write_Newline   CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        //Gathers the treetop node.
        //API.output_Newline(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }


    int write_Text(std::ifstream* p_File)
    {
        std::cout << "\n --> write_Text   CONSTRUCT_ID   TEXT |";

        std::string tmp_In = "";
        std::string tmp_In_Full = "";
        int tmp_Count = 0;

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        bool flg_Gather_Input = true;

        while (flg_Gather_Input)
        {
            tmp_In = "";
            *p_File >> tmp_In;

            std::cout << " " << tmp_In << " |";

            if (tmp_In == "/end/")
            {
                flg_Gather_Input = false;
                continue;
            }

            if (tmp_In != "")
            {

                tmp_Count++;

                if (tmp_In_Full != "")
                {
                    tmp_In_Full = tmp_In_Full + " " + tmp_In;
                }
                else
                {
                    tmp_In_Full = tmp_In;
                }
            }

            if (flg_Gather_Input)
            {
                flg_Gather_Input = (!p_File->eof());
            }
        }

        //API.write_Text(tmp_Construct, tmp_In_Full);


        //std::cout << " [|x|]";

        return 1;
    }



    void write_String(std::ifstream* p_File)
    {
        std::cout << "\n --> write_String |";

        std::string tmp_Output_FName;
        *p_File >> tmp_Output_FName;

        std::string tmp_Write = "";
        *p_File >> tmp_Write;

        std::cout << " String: " << tmp_Write << " |";
        std::cout << " Output_FName: " << tmp_Output_FName << " |";

        std::ofstream tmp_Output_File(tmp_Output_FName, std::ios::trunc);

        if (!tmp_Output_File.is_open())
        {
            std::cerr << "\nError: Unable to open file " << tmp_Output_FName << " for writing.";
            return;
        }

        tmp_Output_File << tmp_Write;

        tmp_Output_File.close();
    }

    int echo(std::ifstream* p_File)
    {
        //---std::cout << "\n --> Echo |\n";

        std::string tmp_In = "";
        std::string tmp_In_Full = "";
        int tmp_Count = 0;

        bool flg_Gather_Input = true;

        while (flg_Gather_Input)
        {
            tmp_In = "";
            *p_File >> tmp_In;

            if (tmp_In == "/end/")
            {
                flg_Gather_Input = false;
                continue;
            }

            if (tmp_In != "")
            {

                tmp_Count++;

                if (tmp_In_Full != "")
                {
                    tmp_In_Full = tmp_In_Full + " " + tmp_In;
                }
                else
                {
                    tmp_In_Full = tmp_In;
                }
            }

            if (flg_Gather_Input)
            {
                flg_Gather_Input = (!p_File->eof());
            }
        }

        std::cout << "\n\n" << tmp_In_Full;
        std::cout << "\n";

        //std::cout << " [|x|]";

        return 1;
    }


    /** The main loop for the neuro-server.

        run

    \retval None This function doesn't return any values.

    Call this to start the server in C++.

    This loop outputs a the message as a means of delay, then checks the flag file, and if anything is found it calls the interpreter.

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int run()
    {
        if (flg_Exit)
        {
            std::cout << "\n --> exit |";
            std::cerr << "\n\n   (o~o) It's been fun anon...goodbye...  \n\n";
            return 1;
        }

        int flg_Direction = 1;
        int tmp_Distance = -250;
        int tmp_MAX = 250;

        std::string tmp_Message = "________________________IDLING________________________";
        std::string tmp_Corrupt_Message = "";

        flg_Direction = 1;
        tmp_Distance = 0;
        while (1)
        {
            for (int cou_Time = 0; cou_Time < 250; cou_Time++)
            {
                tmp_Distance += flg_Direction;

                if (tmp_Distance >= tmp_MAX) { flg_Direction = -1; }
                if (tmp_Distance <= (tmp_MAX * -1)) { flg_Direction = 1; }

				/*
                for (int cou_Index = 0; cou_Index < (tmp_Message.length() + 9); cou_Index++)
                {
                    std::cout << char(8);
                }
				*/
				std::cout << "\r";

                tmp_Corrupt_Message = tmp_Message;

                for (int cou_Index = 0; cou_Index < tmp_Distance; cou_Index++)
                {
                    tmp_Corrupt_Message[rand() % tmp_Message.length()] = char((std::rand() % 32) + 32);
                }

                std::cout << "STANDBY[" << tmp_Corrupt_Message << "]" << Tick << "   ";
            }

            int tmp_Result = check_Control_Panel_Flag();

            //Check the control panel for an update
            if (tmp_Result)
            {
                //std::cout << "\n ~~~Calling control interpreter:";

                if (execute_Control_Panel_Buffer() == -1)
                {
                    std::cout << "\n --> exit |";
                    std::cerr << "\n\n   (o~o) It's been fun anon...goodbye...  \n\n";

                    return 1;
                }

                std::cout << "\n\n (o.O)";

                std::cout << tmp_Message;
				
				
				//Set the control flag finished file to 1
				std::ofstream tmp_CFFF("./Control_Panel_Finished.ssv", std::ios::trunc);
				tmp_CFFF << "1";
				tmp_CFFF.close();
				
				std::cout << "\n\n";
            }
			
			
			tmp_Result = 0;
			tmp_Result = check_Update_Flag();
			
			//If the flag is set to run then execute the system update script
			if (tmp_Result)
			{
				tmp_Result = interpret_File("./Scripts/update.txt");
				if (tmp_Result == 0)
				{
					std::cerr << "\n\n   Error: Failed to execute system update script!\n\n";
				}
				Tick_Processor++;
				
				log_Current_Status();
				log_Current_Onions();
				log_Deviation_Mapping();
				log_Current_Projection();
				API.write_System_State_Snapshot();
				//Does this in eval_Traces
				//API.write_Bulk(1, "/System_State_Files/bulk.ssv");
				
				std::cout << "\n\n";
			}
            Tick++;
			
			//log_IO_History(); was moved to gather_Input, so everytime that is called it will log the IO, in the future this may need fixed.
        }
    }
	
	// Append current afferent/efferent values as a time-series.
	// afferent_log.ssv: Tick A0 A1 A2 ...
	// efferent_log.ssv: Tick E0 E1 E2 ...
	void log_IO_History()
	{
		// --- Afferent log ---
		{
			std::ofstream af_log("./System_State_Files/afferent_log.ssv", std::ios::app);
			if (!af_log.is_open())
			{
				std::cerr << "\n[log_IO_History] Unable to open System_State_Files/afferent_log.ssv\n";
			}
			else
			{
				af_log.setf(std::ios::fixed);
				af_log.precision(6);

				af_log << API.Tick_Count;
				for (int i = 0; i < API.IO.Afferent_Count; ++i)
				{
					double val = 0.0;
					if (API.IO.Afferent && API.IO.Afferent[i])
					{
						val = API.IO.Afferent[i]->get_Value_Data();
					}
					af_log << " " << val;
				}
				af_log << "\n";
			}
		}

		// --- Efferent log ---
		{
			std::ofstream ef_log("./System_State_Files/efferent_log.ssv", std::ios::app);
			if (!ef_log.is_open())
			{
				std::cerr << "\n[log_IO_History] Unable to open System_State_Files/efferent_log.ssv\n";
			}
			else
			{
				ef_log.setf(std::ios::fixed);
				ef_log.precision(6);

				ef_log << Tick;
				for (int j = 0; j < API.IO.Efferent_Count; ++j)
				{
					double val = 0.0;
					if (API.IO.Efferent && API.IO.Efferent[j])
					{
						val = API.IO.Efferent[j]->get_Value_Data();
					}
					ef_log << " " << val;
				}
				ef_log << "\n";
			}
		}
	}

	
    // Snapshot of current projected trajectory (RF = 0 by default).
    // Each row: chrono_index  chan0  chan1  ...  chanM
    void log_Current_Projection(int p_RF = 0)
    {
        std::vector<std::vector<u_Data>> tmp_Projection = API.get_Current_Projection(p_RF);

        std::ofstream projFile("./System_State_Files/projection.ssv", std::ios::app);
        if (!projFile.is_open())
        {
            std::cerr << "\n[log_Current_Projection] Unable to open ./System_State_Files/projection.ssv\n";
            return;
        }

        for (size_t cou_C = 0; cou_C < tmp_Projection.size(); ++cou_C)
        {
            projFile << cou_C; // chrono index

            for (size_t cou_R = 0; cou_R < tmp_Projection[cou_C].size(); ++cou_R)
            {
                projFile << " " << tmp_Projection[cou_C][cou_R].D;
            }

            projFile << "\n";
        }
    }
	
	// Snapshot of current deviation mapping per afferent.
    // Format (single row):
    //   0  dev[0] dev[1] ... dev[N-1]
    void log_Deviation_Mapping()
    {
        std::vector<double> tmp_Deviation = API.get_Current_Deviation_Set();

        std::ofstream devFile("./System_State_Files/deviation_mapping.ssv", std::ios::app);
        if (!devFile.is_open())
        {
            std::cerr << "\n[log_Deviation_Mapping] Unable to open ./System_State_Files/deviation_mapping.ssv\n";
            return;
        }

        // Use first column as an index (0) so DVI.2-style tools can treat it as a "row id".
        devFile << 0;
        for (size_t i = 0; i < tmp_Deviation.size(); ++i)
        {
            devFile << " " << tmp_Deviation[i];
        }
        devFile << "\n";
    }

	
	void log_Current_Onions()
	{
		std::ofstream onion_File("System_State_Files/onions.ssv", std::ios::trunc);
		if (!onion_File.is_open())
		{
			std::cerr << "\nUnable to open System_State_Files/onions.ssv for writing.\n";
			return;
		}

		// Optional: make the numbers a bit prettier / stable
		onion_File.setf(std::ios::fixed);
		onion_File.precision(4);

		// One block per afferent
		for (int a = 0; a < API.IO.Afferent_Count; a++)
		{
			if (!API.IO.Afferent || !API.IO.Afferent[a]) { continue; }

			c_Granulator &G = API.IO.Afferent[a]->Granulator;

			// If this afferent has no bands configured yet, skip it
			if (G.Count <= 0 || !G.Bottom || !G.Top || !G.Mid) { continue; }

			for (int band = 0; band < G.Count; band++)
			{
				double bottom = G.Bottom[band];
				double top    = G.Top[band];
				double mid    = G.Mid[band];

				// domain, afferent id, band index, bottom, top, mid
				onion_File << "ONION "
						   << "A" << a << " "
						   << band << " "
						   << bottom << " "
						   << top    << " "
						   << mid    << " \n";
			}
		}

		onion_File.close();
	}

		
	void log_Current_Status()
	{
		std::ofstream status_File("System_State_Files/status.ssv", std::ios::trunc);
		if (!status_File.is_open())
		{
			std::cout << "\nUnable to open System_State_Files/status.ssv for writing.\n";
			std::cerr << "\nUnable to open System_State_Files/status.ssv for writing.\n";
			return;
		}

		// --- Engine-level status ---
		status_File << "ENGINE Session_Tick "   << Tick           << " \n";
		status_File << "ENGINE Processor_Tick " << API.Tick_Count << " \n";

		// Optional: a couple of simple engine flags
		status_File << "ENGINE Run_Update " << (flg_Run_Update ? 1 : 0) << " \n";
		status_File << "ENGINE Exit_Flag "  << (flg_Exit       ? 1 : 0) << " \n";

		// --- I/O summary ---
		status_File << "IO Afferent_Count " << API.IO.Afferent_Count << " \n";
		status_File << "IO Efferent_Count " << API.IO.Efferent_Count << " \n";

		// --- Per-afferent status: value / goal / deviation ---
		for (int cou_A = 0; cou_A < API.IO.Afferent_Count; cou_A++)
		{
			double val  = 0.0;
			double goal = 0.0;
			double dev  = 0.0;

			if (API.IO.Afferent && API.IO.Afferent[cou_A])
			{
				val  = API.IO.Afferent[cou_A]->get_Value_Data();
				goal = API.IO.Afferent[cou_A]->get_Goal();
				dev  = API.IO.Afferent[cou_A]->get_Value_Deviation();
			}

			status_File << "AFFERENT A" << cou_A << "_Value " << val  << " \n";
			status_File << "AFFERENT A" << cou_A << "_Goal "  << goal << " \n";
			status_File << "AFFERENT A" << cou_A << "_Dev "   << dev  << " \n";
		}

		// --- Per-efferent status: current value / drive ---
		for (int cou_E = 0; cou_E < API.IO.Efferent_Count; cou_E++)
		{
			double val = 0.0;

			if (API.IO.Efferent && API.IO.Efferent[cou_E])
			{
				val = API.IO.Efferent[cou_E]->get_Value_Data();
			}

			status_File << "EFFERENT E" << cou_E << "_Value " << val << " \n";
		}

    // --- NT4 / construct-network status ---
    status_File << "NT4 Chrono_Depth "
                << API.TSG.Chrono_Depth << " \n";

    status_File << "NT4 Raw_Depth "
                << API.TSG.Raw_Depth << " \n";

    status_File << "NT4 Node_Count "
                << API.TSG.NT4_Core.Base.Nodes.Node_Count << " \n";

    bool tmp_Bored =
        (API.Previous_Node_Count ==
         int(API.TSG.NT4_Core.Base.Nodes.Node_Count));

    status_File << "NT4 Bored " << (tmp_Bored ? 1 : 0) << " \n";

    // --- External flags (peek without clearing) ---
    {
        std::string flagValue = "missing";
        std::ifstream flagFile("Control_Panel_Flag.ssv");
        if (flagFile.is_open())
        {
            flagValue.clear();
            flagFile >> flagValue;
            if (flagValue.empty()) { flagValue = "0"; }
        }
        status_File << "FLAGS Control_Panel_Flag " << flagValue << " \n";
    }

    {
        std::string flagValue = "missing";
        std::ifstream flagFile("Update_Flag.ssv");
        if (flagFile.is_open())
        {
            flagValue.clear();
            flagFile >> flagValue;
            if (flagValue.empty()) { flagValue = "0"; }
        }
        status_File << "FLAGS Update_Flag " << flagValue << " \n";
    }

    status_File.close();
}

	
    void rcon()
    {

    }


    void register_Afferent(std::ifstream* p_File)
    {
        std::cout << "\n --> register_Afferent |";

        int tmp_Index = API.register_Afferent();
    }

    void register_Efferent(std::ifstream* p_File)
    {
        std::cout << "\n --> register_Efferent |";

        int tmp_Index = API.register_Efferent();
    }

    void setup_Gaia(std::ifstream* p_File)
    {
        std::cout << "\n --> setup_Gaia |";

        int tmp_Chrono_Depth = 0, tmp_A_Depth = 0, tmp_E_Depth = 0, tmp_Start = 0, tmp_End = 0, tmp_Step = 0;
        *p_File >> tmp_Chrono_Depth >> tmp_A_Depth >> tmp_E_Depth >> tmp_Start >> tmp_End >> tmp_Step;
        std::cout << " Chrono_Depth: " << tmp_Chrono_Depth << " A_Depth: " << tmp_A_Depth << " E_Depth: " << tmp_E_Depth << " Start: " << tmp_Start << " End: " << tmp_End << " Step: " << tmp_Step;

        // Ensure step is not zero to avoid infinite loop
        if (tmp_Step == 0)
        {
            std::cerr << "\nError: Step size (p_Step) cannot be zero.";
            return;
        }

        // Setup afferent connections
        for (int cou_IO = 0; cou_IO < tmp_A_Depth; cou_IO++)
        {
            /*-*/std::cout << "\n <A[" << cou_IO << "]> " << API.register_Afferent();

            //Create the IO file
            std::ofstream tmp_F;
            std::string tmp_FName = "./IO_Files/A/" + std::to_string(cou_IO) + ".a.ssv";
            tmp_F.open(tmp_FName, std::ios::app);
            //tmp_F << "\n";
            tmp_F.close();

            float tmp_Middle = float(tmp_Start + tmp_End) / 2;

            // Dynamically compute granulation based on p_Start, p_End, and p_Step
            for (int granulation_value = 0; granulation_value <= tmp_End; granulation_value += tmp_Step)
            {
                API.add_Afferent_Granulation((tmp_Start - granulation_value), (tmp_Start + granulation_value), cou_IO);
                std::cout << "\n add_Afferent_Granulation: Low: " << (tmp_Start - granulation_value) << " High: " << (tmp_Start + granulation_value);
            }
        }

        // Setup efferent connections
        for (int cou_IO = 0; cou_IO < tmp_E_Depth; cou_IO++)
        {
            /*-*/std::cout << "\n <E[" << cou_IO << "]> " << API.register_Efferent();

            //Create the IO file
            std::ofstream tmp_F;
            std::string tmp_FName = "./IO_Files/E/" + std::to_string(cou_IO) + ".e.ssv";
            tmp_F.open(tmp_FName, std::ios::app);
            //tmp_F << "\n";
            tmp_F.close();
        }

        // Initialize the system with the given chrono depth
        API.init(tmp_Chrono_Depth);
    }
	
	
	void set_Afferent_Bands(std::ifstream* p_File)
	{
		std::cout << "\n --> set_Afferent_Bands |";

		int tmp_A_Index = 0;
		int tmp_Start   = 0;
		int tmp_End     = 0;
		int tmp_Step    = 0;

		// Read: Afferent index, goal/center, end range, step
		*p_File >> tmp_A_Index >> tmp_Start >> tmp_End >> tmp_Step;

		std::cout << " A_Index: " << tmp_A_Index
				  << " Start: "  << tmp_Start
				  << " End: "    << tmp_End
				  << " Step: "   << tmp_Step;

		// Ensure step is not zero to avoid infinite loop
		if (tmp_Step == 0)
		{
			std::cerr << "\nError: Step size (tmp_Step) cannot be zero.";
			return;
		}

		// Optional: sanity check on afferent index if API exposes depth
		// int tmp_A_Depth = API.get_Afferent_Depth();
		// if (tmp_A_Index < 0 || tmp_A_Index >= tmp_A_Depth)
		// {
		//     std::cerr << "\nError: Afferent index out of range: " << tmp_A_Index;
		//     return;
		// }

		// Set the concrete goal for this afferent (center of the onion)
		// Requires you to implement API.set_Afferent_Goal(...)
		API.set_Afferent_Goal(tmp_A_Index, tmp_Start);

		// Wipe existing bands for this afferent so we can rebuild the onion
		// Requires you to implement API.wipe_Afferent_Bands(...)
		API.wipe_Afferent_Bands(tmp_A_Index);

		// Dynamically compute granulation based on tmp_Start, tmp_End, and tmp_Step
		for (int granulation_value = 0; granulation_value <= tmp_End; granulation_value += tmp_Step)
		{
			int tmp_Low  = tmp_Start - granulation_value;
			int tmp_High = tmp_Start + granulation_value;

			API.add_Afferent_Granulation(tmp_Low, tmp_High, tmp_A_Index);

			std::cout << "\n add_Afferent_Granulation: A[" << tmp_A_Index
					  << "] Low: " << tmp_Low
					  << " High: " << tmp_High;
		}
	}



    void shift_Data(std::ifstream* p_File, bool p_Echo = true)
    {
        
        if (p_Echo) { std::cout << "\n --> shift_Data |"; }
        API.shift_Data();
    }

    void set_Afferent_Value(std::ifstream* p_File, bool p_Echo = true)
    {
        if (p_Echo) { std::cout << "\n --> set_Afferent_Value |"; }

        int tmp_Index = 0;

        *p_File >> tmp_Index;

        
        if (p_Echo) { std::cout << " " << tmp_Index << " |"; }

        float tmp_Value = 0;

        *p_File >> tmp_Value;

        
        if (p_Echo) { std::cout << " " << tmp_Value << " |"; }

        API.set_Afferent_Value(tmp_Index, tmp_Value);
    }
	

    void set_Efferent_Value(std::ifstream* p_File, bool p_Echo = true)
    {
        
        if (p_Echo) { std::cout << "\n --> set_Efferent_Value |"; }

        int tmp_Index = 0;

        *p_File >> tmp_Index;

        
        if (p_Echo) { std::cout << " " << tmp_Index << " |"; }

        float tmp_Value = 0;

        *p_File >> tmp_Value;

        
        if (p_Echo) { std::cout << " " << tmp_Value << " |"; }

        API.set_Efferent_Value(tmp_Index, tmp_Value);
    }

    //Potentially make this shift the system as well, iterate GaiaOS. 
    void gather_Input(std::ifstream* p_File, bool p_Echo = true)
    {
        if (p_Echo) { std::cout << "\n --> gather_Input |"; }

        std::ifstream tmp_F;

        float tmp_Input = 0.0;

        for (int cou_A = 0; cou_A < API.get_Afferent_Count(); cou_A++)
        {
            //API.set_Afferent_Value(cou_S, API.get);

            //Create the IO file
            std::ifstream tmp_OF;
            std::string tmp_FName = "./IO_Files/A/" + std::to_string(cou_A) + ".a.ssv";
            tmp_OF.open(tmp_FName);
            
            tmp_OF >> tmp_Input;

            API.set_Afferent_Value(cou_A, tmp_Input);

            tmp_OF.close();

        }

        for (int cou_E = 0; cou_E < API.get_Efferent_Count(); cou_E++)
        {
            //API.set_Efferent_Value(cou_A, p_Map->get_Actuator_State(cou_A));
            //Create the IO file
            std::ifstream tmp_OF;
            std::string tmp_FName = "./IO_Files/E/" + std::to_string(cou_E) + ".e.ssv";
            tmp_OF.open(tmp_FName);

            tmp_OF >> tmp_Input;

            API.set_Efferent_Value(cou_E, tmp_Input);

            tmp_OF.close();
        }
		log_IO_History();
    }

    void gather_Output(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Output |";

        std::cout << "\n Efferent_Count: " << API.get_Efferent_Count();

        //Gather outputs from internal state to present in the output files.
        for (int cou_S = 0; cou_S < API.get_Efferent_Count(); cou_S++)
        {
            std::cout << "\n Signal[" << cou_S << "]: " << API.get_Output_Signals(cou_S);

            std::ofstream tmp_OFile;
            std::string tmp_FName = "./IO_Files/E/" + std::to_string(cou_S) + ".e.ssv";
            tmp_OFile.open(tmp_FName, std::ios::ate);
            tmp_OFile << API.get_Output_Signals(cou_S);
            tmp_OFile.close();
        }
    }

    void eval(std::ifstream* p_File)
    {
        std::cout << "\n --> eval |";

        std::string tmp_FName = "";

        *p_File >> tmp_FName;

        std::cout << " " << tmp_FName << " |";

        float tmp_Score_Threshold_Level = 0;

        *p_File >> tmp_Score_Threshold_Level;

        std::cout << " " << tmp_Score_Threshold_Level << " |";

        API.eval(tmp_FName, tmp_Score_Threshold_Level);
    }

    void write_Bulk(std::ifstream* p_File)
    {
        std::cout << "\n --> write_Bulk |";

        std::string tmp_FName = 0;

        *p_File >> tmp_FName;

        std::cout << " " << tmp_FName << " |";

        int tmp_Tick = 0;

        *p_File >> tmp_Tick;

        std::cout << " " << tmp_Tick << " |";

        API.write_Bulk(tmp_FName, tmp_Tick);
    }

    void get_Output_Signals(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Output_Signals |";

        int tmp_Index = 0;

        *p_File >> tmp_Index;

        //std::cout << " " << tmp_Index << " |";

        std::cout << " Signal[" << tmp_Index << "]: " << API.get_Output_Signals(tmp_Index);
    }

    void encode(std::ifstream* p_File, bool p_Echo = true)
    {
        if (p_Echo) { std::cout << "\n --> encode |"; }
        API.encode();
    }









    void write_Deviation_Mapping(std::ifstream* p_File)
    {
        std::cout << "\n --> write_Deviation_Mapping |";
        //API.write_Deviation_Mapping();
    }

    void set_TSG_Prediction_Params(std::ifstream* p_File)
    {
        std::cout << "\n --> set_TSG_Prediction_Params |";
        //API.set_TSG_Prediction_Params();
    }

    void set_TSG_Deviation_Params(std::ifstream* p_File)
    {
        std::cout << "\n --> set_TSG_Deviation_Params |";
        //API.set_TSG_Deviation_Params();
    }

    void init(std::ifstream* p_File)
    {
        std::cout << "\n --> init |";

        int tmp_Chrono_Depth = 0;

        *p_File >> tmp_Chrono_Depth;

        std::cout << " " << tmp_Chrono_Depth << " |";

        API.init(tmp_Chrono_Depth);
    }



    /*

    void add_Afferent_Granulation(std::ifstream* p_File)
    {
        std::cout << "\n --> add_Afferent_Granulation |";
        API.add_Afferent_Granulation();
    }

    void set_Depth(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Depth |";
        API.set_Depth();
    }

    void set_Chrono_Depth(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Chrono_Depth |";
        API.set_Chrono_Depth();
    }

    void update_MSC_Depth(std::ifstream* p_File)
    {
        std::cout << "\n --> update_MSC_Depth |";
        API.update_MSC_Depth();
    }

    void get_Chrono_Depth(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Chrono_Depth |";
        API.get_Chrono_Depth();
    }

    void get_Current_Projection(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Current_Projection |";
        API.get_Current_Projection();
    }

    void output_Current_Projection(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Current_Projection |";
        API.output_Current_Projection();
    }

    void get_Random_Raw_Projection(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Random_Raw_Projection |";
        API.get_Random_Raw_Projection();
    }
    void output_Deviation_Mapping(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Deviation_Mapping |";
        API.output_Deviation_Mapping();
    }

    void get_Current_Deviation_Set(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Current_Deviation_Set |";
        API.get_Current_Deviation_Set();
    }

    void get_Deviation_Set(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Deviation_Set |";
        API.get_Deviation_Set();
    }

    void copy_Deviation(std::ifstream* p_File)
    {
        std::cout << "\n --> copy_Deviation |";
        API.copy_Deviation();
    }

    void copy_Deviation_Index(std::ifstream* p_File)
    {
        std::cout << "\n --> copy_Deviation_Index |";
        API.copy_Deviation_Index();
    }

    void output_TSG(std::ifstream* p_File)
    {
        std::cout << "\n --> output_TSG |";
        API.output_TSG();
    }

    void output_NNet(std::ifstream* p_File)
    {
        std::cout << "\n --> output_NNet |";
        API.output_NNet();
    }

    void output_AE(std::ifstream* p_File)
    {
        std::cout << "\n --> output_AE |";
        API.output_AE();
    }
    */

    //Output node network
    void output_NNet(std::ifstream* p_File)
    {
        std::cout << "\n --> output_NNet |";

        API.output_NNet();
    }
    

    //Output node network
    void output_Scaffolds(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Scaffolds |";

        API.output_Scaffolds();
    }

    //Output output_TSG
    void output_TSG(std::ifstream* p_File)
    {
        std::cout << "\n --> output_TSG |";

        API.output_TSG();
    }

    //Output output_AE
    void output_AE(std::ifstream* p_File)
    {
        std::cout << "\n --> output_AE |";

        API.output_AE();
    }

    //Output output_Current_Projection
    void output_Current_Projection(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Current_Projection |";

        int tmp_RF = 0;

        *p_File >> tmp_RF;

        std::cout << " " << tmp_RF << " |";

        API.output_Current_Projection(tmp_RF);
    }

    //Output output_Deviation_Mapping
    void output_Deviation_Mapping(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Deviation_Mapping |";

        API.output_Deviation_Mapping();
    }

    //Output output_Backpropagated_Symbols
    void output_Backpropagated_Symbols_Float(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Backpropagated_Symbols_Float |";

        API.TSG.NT4_Core.output_Backpropagated_Symbols(2);
    }
    
private:

    int eval_Command(std::string p_Command, std::ifstream* p_File)
    {
        //==--  Basic Command List  --=//
        // 
        //Meta commands for the engine rather than the nodes and internal structures.
        if (p_Command == "exit") { return -1; }
        if (p_Command == "/?") { help_Text(); return 1; }
        if (p_Command == "help") { help_Text(); return 1; }
        if (p_Command == "start_engine") { flg_Run_Update = true; return 1; }
        if (p_Command == "stop_engine") { flg_Run_Update = false; return 1; }

        //Data Loading and Preparation:
        //if (p_Command == "reset_Input") { reset_Input(p_File); return 1; }

        //Saving and loading the whole thing.
        //if (p_Command == "save") { save(p_File); return 1; }
        //if (p_Command == "load") { load(p_File); return 1; }

        //File Output:
        if (p_Command == "clear_Output") { clear_Output(p_File); return 1; }
        if (p_Command == "write_Newline") { output_Newline(p_File); return 1; }
        if (p_Command == "write_Text") { write_Text(p_File); return 1; }
        if (p_Command == "write_String") { write_String(p_File); return 1; }
        if (p_Command == "echo") { echo(p_File); return 1; }

        // Registering inputs/outputs:
        if (p_Command == "setup_Gaia") { setup_Gaia(p_File); return 1; }
        if (p_Command == "register_afferent") { register_Afferent(p_File); return 1; }
        if (p_Command == "register_efferent") { register_Efferent(p_File); return 1; }

        //Commands for the homeostasis module.
        if (p_Command == "@shift_Data") { shift_Data(p_File, false); return 1; }
        if (p_Command == "shift_Data") { shift_Data(p_File); return 1; }
        if (p_Command == "set_Afferent_Bands") { set_Afferent_Bands(p_File); return 1; }
        if (p_Command == "set_Afferent_Value") { set_Afferent_Value(p_File); return 1; }
        if (p_Command == "set_Efferent_Value") { set_Efferent_Value(p_File); return 1; }
        if (p_Command == "@set_Afferent_Value") { set_Afferent_Value(p_File, false); return 1; }
        if (p_Command == "@set_Efferent_Value") { set_Efferent_Value(p_File, false); return 1; }
        if (p_Command == "@gather_Input") { gather_Input(p_File, false); return 1; }
        if (p_Command == "gather_Input") { gather_Input(p_File); return 1; }
        if (p_Command == "gather_Output") { gather_Output(p_File); return 1; }
        if (p_Command == "eval") { eval(p_File); return 1; }
        if (p_Command == "write_Bulk") { write_Bulk(p_File); return 1; }
        if (p_Command == "get_Output_Signals") { get_Output_Signals(p_File); return 1; }
        if (p_Command == "@encode") { encode(p_File, false); return 1; }
        if (p_Command == "encode") { encode(p_File); return 1; }

        if (p_Command == "output_AE") { output_AE(p_File); return 1; }
        if (p_Command == "output_NNet") { output_NNet(p_File); return 1; }
        if (p_Command == "output_TSG") { output_TSG(p_File); return 1; }
        if (p_Command == "output_Scaffolds") { output_Scaffolds(p_File); return 1; }
        if (p_Command == "output_Backpropagated_Symbols_Float") { output_Backpropagated_Symbols_Float(p_File); return 1; }

        if (p_Command == "output_Current_Projection") { output_Current_Projection(p_File); return 1; }
        if (p_Command == "output_Deviation_Mapping") { output_Deviation_Mapping(p_File); return 1; }
        if (p_Command == "output_Backpropagated_Symbols_Float") { output_Backpropagated_Symbols_Float(p_File); return 1; }
        
        /*
        if (p_Command == "getOutput_Signals") { get_Output_Signals(p_File); return 1; }
        if (p_Command == "write_Deviation_Mapping") { write_Deviation_Mapping(p_File); return 1; }
        if (p_Command == "set_TSG_Prediction_Params") { set_TSG_Prediction_Params(p_File); return 1; }
        if (p_Command == "set_TSG_Deviation_Params") { set_TSG_Deviation_Params(p_File); return 1; }
        if (p_Command == "init") { init(p_File); return 1; }
        if (p_Command == "init_TSG") { init_TSG(p_File); return 1; }
        if (p_Command == "encode") { encode(p_File); return 1; }
        if (p_Command == "evaluate_Traces") { evaluate_Traces(p_File); return 1; }
        if (p_Command == "eval") { eval(p_File); return 1; }
        if (p_Command == "register_Afferent") { register_Afferent(p_File); return 1; }
        if (p_Command == "register_Efferent") { register_Efferent(p_File); return 1; }
        if (p_Command == "add_Afferent_Granulation") { add_Afferent_Granulation(p_File); return 1; }
        if (p_Command == "set_Depth") { set_Depth(p_File); return 1; }
        if (p_Command == "set_Chrono_Depth") { set_Chrono_Depth(p_File); return 1; }
        if (p_Command == "update_MSC_Depth") { update_MSC_Depth(p_File); return 1; }
        if (p_Command == "get_Chrono_Depth") { get_Chrono_Depth(p_File); return 1; }
        if (p_Command == "get_Current_Projection") { get_Current_Projection(p_File); return 1; }
        if (p_Command == "output_Current_Projection") { output_Current_Projection(p_File); return 1; }
        if (p_Command == "get_Random_Raw_Projection") { get_Random_Raw_Projection(p_File); return 1; }
        if (p_Command == "output_Deviation_Mapping") { output_Deviation_Mapping(p_File); return 1; }
        if (p_Command == "get_Current_Deviation_Set") { get_Current_Deviation_Set(p_File); return 1; }
        if (p_Command == "get_Deviation_Set") { get_Deviation_Set(p_File); return 1; }
        if (p_Command == "copy_Deviation") { copy_Deviation(p_File); return 1; }
        if (p_Command == "copy_Deviation_Index") { copy_Deviation_Index(p_File); return 1; }
        if (p_Command == "output_TSG") { output_TSG(p_File); return 1; }
        if (p_Command == "output_NNet") { output_NNet(p_File); return 1; }
        if (p_Command == "output_AE") { output_AE(p_File); return 1; }
        */


        //See if they submitted a command, these scripts are retrieved from the ./Scripts/ dir.
        std::string tmp_Command = "./Scripts/" + p_Command;
        int tmp_PF = interpret_File(tmp_Command);

        if (tmp_PF == 0)
        {
            std::string tmp_Command = "./Scripts/" + p_Command + ".txt";
            interpret_File(tmp_Command);
        }

        return 0;
    }

	void help_Text() 
	{
		using std::cout;

		cout << "\n";
		cout << "================================================================================\n";
		cout << "                               G A I A   S H E L L                              \n";
		cout << "================================================================================\n";
		cout << "File-driven control loop for registering IO, gathering values, encoding,        \n";
		cout << "evaluating (decision/inference), and mirroring efferent decisions to disk.      \n";
		cout << "Unknown tokens are treated as script includes from ./Scripts/ (and .txt).       \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "BASICS                                                                          \n";
		cout << "  Control_Panel.ssv          : Script buffer (tokens separated by whitespace).  \n";
		cout << "  Control_Panel_Flag.ssv     : Write '1' to request execution.                  \n";
		cout << "  Control_Panel_Finished.ssv : Engine writes '1' when execution is complete.    \n";
		cout << "  IO_Files/A/<i>.a.ssv       : Afferent inputs (floats).                        \n";
		cout << "  IO_Files/E/<j>.e.ssv       : Efferent outputs (integers; 1 or -1).           \n";
		cout << "  GaiaTesting/<label>.*.ssv  : Eval analytics (trace counts, etc.).            \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "CORE COMMANDS (tokens)                                                          \n";
		cout << "  help | /?                               Show this help                        \n";
		cout << "  exit                                    Stop interpreter (returns -1)         \n";
		cout << "\n";
		cout << "  setup_Gaia  <Chrono> <A> <E> <Start> <End> <Step>                             \n";
		cout << "      Registers A afferents & E efferents, creates IO files, sets granulation   \n";
		cout << "      rings centered at Start expanding by Step up to End, then init Chrono.    \n";
		cout << "\n";
		cout << "  register_afferent                        Add one afferent (no file creation)  \n";
		cout << "  register_efferent                        Add one efferent (no file creation)  \n";
		cout << "\n";
		cout << "  @gather_Input | gather_Input             Read IO_Files into internal state     \n";
		cout << "  @shift_Data  | shift_Data                Advance buffers / chronology          \n";
		cout << "  @encode      | encode                    Encode current snapshot               \n";
		cout << "  eval <label> <threshold>                 Decide outputs; set Output_Signals    \n";
		cout << "  gather_Output                            Mirror Output_Signals to E-files      \n";
		cout << "  get_Output_Signals <idx>                 Print current signal (1 or -1)        \n";
		cout << "\n";
		cout << "  write_String <path> <token>              Truncate file and write one token     \n";
		cout << "  echo <... /end/>                         Print text to console                 \n";
		cout << "\n";
		cout << "DIAGNOSTICS (console dumps)                                                     \n";
		cout << "  output_AE                                IO map                                \n";
		cout << "  output_TSG                               Time-series internals                 \n";
		cout << "  output_NNet                              Node network                          \n";
		cout << "  output_Scaffolds                         Scaffold structures                   \n";
		cout << "  output_Current_Projection <RF>           Current projection for RF             \n";
		cout << "  output_Deviation_Mapping                 Per-afferent deviation lanes          \n";
		cout << "  output_Backpropagated_Symbols_Float      Float backprop symbols                \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "EVAL THRESHOLD                                                                  \n";
		cout << "  eval <label> <threshold> : After normalization, an efferent is ON if its      \n";
		cout << "  composite score > (max composite * threshold). Typical values: 1.0–1.5.       \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "EXAMPLE — BASIC SETUP (2 afferents, 2 efferents)                                \n";
		cout << "  Scripts/autoexec.ssv                                                          \n";
		cout << "    setup_Gaia  8  2  2  50  100  25                                            \n";
		cout << "    output_AE                                                                    \n";
		cout << "\n";
		cout << "  First run: engine loads autoexec at boot, creates:                            \n";
		cout << "    IO_Files/A/0.a.ssv, IO_Files/A/1.a.ssv, IO_Files/E/0.e.ssv, IO_Files/E/1.e.ssv\n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "EXAMPLE — MANUAL CONTROL (single tick)                                          \n";
		cout << "  # Seed inputs                                                                 \n";
		cout << "  write_String IO_Files/A/0.a.ssv 42                                            \n";
		cout << "  write_String IO_Files/A/1.a.ssv 60                                            \n";
		cout << "\n";
		cout << "  # Run one inference pass                                                      \n";
		cout << "  @gather_Input                                                                 \n";
		cout << "  @shift_Data                                                                   \n";
		cout << "  @encode                                                                       \n";
		cout << "  eval loop 1.2                                                                 \n";
		cout << "  gather_Output                                                                 \n";
		cout << "\n";
		cout << "  # Check a decision                                                            \n";
		cout << "  get_Output_Signals 0                                                          \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "EXAMPLE — CONTINUOUS OPERATION (script include + flag handshake)                \n";
		cout << "  Scripts/loop.ssv                                                              \n";
		cout << "    @gather_Input                                                               \n";
		cout << "    @shift_Data                                                                 \n";
		cout << "    @encode                                                                     \n";
		cout << "    eval loop 1.2                                                               \n";
		cout << "    gather_Output                                                               \n";
		cout << "\n";
		cout << "  Each cycle, external controller writes:                                       \n";
		cout << "    Control_Panel.ssv      :  loop.ssv                                          \n";
		cout << "    Control_Panel_Flag.ssv :  1                                                 \n";
		cout << "  Engine runs the script, then writes '1' to Control_Panel_Finished.ssv.        \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "NOTES / FOOTGUNS                                                                \n";
		cout << "  • gather_Output appends; your hardware reader should consume the last value   \n";
		cout << "    or you can add a truncate-style writer for snapshot semantics.              \n";
		cout << "  • Unknown tokens are treated as ./Scripts/<token> or ./Scripts/<token>.txt.   \n";
		cout << "  • @-prefixed commands are silent variants (reduced console noise).            \n";
		cout << "--------------------------------------------------------------------------------\n";
		cout << "QUICK START                                                                     \n";
		cout << "  1) Put autoexec in ./Scripts/ (see BASIC SETUP).                              \n";
		cout << "  2) Boot engine; verify IO files exist.                                        \n";
		cout << "  3) Fill A-files with sensor values, run loop.ssv via flag handshake.          \n";
		cout << "  4) Read E-files and actuate hardware accordingly.                             \n";
		cout << "================================================================================\n";
		cout << "\n";
	}

};

/** @}*/