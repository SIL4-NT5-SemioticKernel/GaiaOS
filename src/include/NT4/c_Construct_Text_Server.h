/** \addtogroup Construct_Text_Server
 *  @{
 */

/** \class c_Construct_Text_Server
    \brief This is a handshake based text interface for the engine. 

    
    File-Based Command Control System for Asynchronous Standalone Neural-Net Engine Control:

    Text-Based Command Execution:
    - Commands are represented as text strings that specify actions or operations to be performed by the system or process.
    - Each command is associated with a specific functionality or operation within the system.

    File I/O:
    - Input and output operations are performed using files as the medium of communication.
    - Commands are read from input files, and outputs, returns, and status indicators are written to output files, though some commands do output to the console.
    - This method facilitates asynchronous communication between systems, as they can read from and write to files independently.

    Handshake Protocol:
    - A handshake protocol is used to establish communication or synchronize actions between systems.
    - In this context, a flag file serves as a mechanism for signaling the readiness or completion of certain operations, namely that the user has written a command sequence to 'Control_Panel.ssv' that is ready to be interpreted..
    - The content of the flag file is used to indicate the status of the control panel, allowing the system to either wait or act accordingly.

    Flag File:
    - A flag file is a small file used to signal the status or state of the system.
    - It typically contains simple data or metadata, such as a single value or indicator, to convey information. In the NT4 text interface it is a boolean value.
    - Flag files are used as synchronization primitives to coordinate activities between systems.

    Control Panel:
    - The control panel represents a set of commands or instructions that can be executed by the system or process.
    - It is stored in a separate file (e.g., Control_Panel.ssv), which contains a sequence of commands or directives to be processed.

    Interpretation and Execution:
    - The system interprets commands read from the control file (e.g., Control_Panel.ssv) and executes them sequentially.
    - Command execution may involve performing operations on data, manipulating the state of the system, or interacting with external resources.

    Status Checking and Control Flow:
    - The system periodically checks the status of a flag file (e.g., Control_Panel_Flag.ssv) to determine if there are new commands to execute or if certain operations have completed.
    - Based on the status indicated by the flag file, the system may initiate specific actions, continue processing, or wait for further instructions.

    Startup and 'autoexec.ssv':
    - For 'booting up' the system interprets the commands found in 'autoexec.ssv' sequentially. This allows for shell scripting a startup sequence.

    Custom Commands:
    - A script file placed in "./Scripts/" can be called by simply entering the filename as you would a command token. A file can call other files so you can create dependency hell for yourself if you wish, but with the added complexity of ML.
    
    \var string const RETURN_FILE defines the file in which returns write their data at this level of the API.
*/

const std::string RETURN_FILE = "./Output/returned.ssv";
class c_Construct_Text_Server
{
    //The construct to hook into.
    NT4::c_Construct_API API;
    
    //Current server tick.
    int Tick;

    //Exit flag to allow for exit after startup if the user puts 'exit' into the autoexec file, needed for CLI capabilities.
    bool flg_Exit;

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




    int eval_Command(std::string p_Command, std::ifstream* p_File)
    {
        //==--  Basic Command List  --=//
        // 
        //Meta commands for the engine rather than the nodes and internal structures.
        if (p_Command == "exit") { return -1; }

        //Data Loading and Preparation:
        if (p_Command == "reset_Input") { reset_Input(p_File); return 1; }
        if (p_Command == "reset_Output") { reset_Output(p_File); return 1; }
        if (p_Command == "set_Input") { set_Input(p_File); return 1; }
        if (p_Command == "set_Input_uint") { set_Input_uint(p_File); return 1; }
        if (p_Command == "set_2D_Input_uint") { set_2D_Input_uint(p_File); return 1; }
        if (p_Command == "set_3D_Input_uint") { set_3D_Input_uint(p_File); return 1; }
        if (p_Command == "load_Input") { load_Input(p_File); return 1; }
        if (p_Command == "load_Input_uint") { load_Input_uint(p_File); return 1; }
        
        if (p_Command == "chrono_Shift") { chrono_Shift(p_File); return 1; }
        if (p_Command == "add_Chrono") { add_Chrono(p_File); return 1; }

        if (p_Command == "round_Up_Input") { round_Up_Input(p_File); return 1; }
        if (p_Command == "pull_From_Lower_Connections") { pull_From_Lower_Connections(p_File); return 1; }
        if (p_Command == "pull_From_Lower_Connection") { pull_From_Lower_Connection(p_File); return 1; }
        if (p_Command == "pull_Chrono_From_Lower_Connection") { pull_Chrono_From_Lower_Connection(p_File); return 1; }

        //Construct manipulation
        if (p_Command == "register_Construct") { register_Construct(p_File); return 1; }
        if (p_Command == "create_Construct_Connection") { create_Construct_Connection(p_File); return 1; }
        if (p_Command == "output_Construct_Connections") { output_Construct_Connections(p_File); return 1; }
        if (p_Command == "pull_From_Upper_Index") { pull_From_Upper_Index(p_File); return 1; }

        //Saving and loading the whole thing.
        if (p_Command == "save") { save(p_File); return 1; }
        if (p_Command == "load") { load(p_File); return 1; }

        //Config
        if (p_Command == "save_Config") { save_Config(p_File); return 1; }
        if (p_Command == "update_Config") { update_Config(p_File); return 1; }

        if (p_Command == "set_Base_Charge") { set_Base_Charge(p_File); return 1; }
        if (p_Command == "set_Modifier_Charge") { set_Modifier_Charge(p_File); return 1; }
        if (p_Command == "set_Action_Potential_Threshold") { set_Action_Potential_Threshold(p_File); return 1; }
        if (p_Command == "set_Charging_Tier") { set_Charging_Tier(p_File); return 1; }

        //Node Manipulations
        if (p_Command == "bp_O") { bp_O(p_File); return 1; }

        //Evaluation
        if (p_Command == "query") { query(p_File); return 1; }

        //Network Encoding
        if (p_Command == "encode") { encode(p_File); return 1; }

        //File Output:
        if (p_Command == "clear_Output") { clear_Output(p_File); return 1; }
        if (p_Command == "write_Newline") { output_Newline(p_File); return 1; }
        if (p_Command == "write_Text"){ write_Text(p_File); return 1; }
        if (p_Command == "echo"){ echo(p_File); return 1; }

        //Network Output Gathering
        if (p_Command == "write_Given_Trace") { gather_Given_Trace(p_File); return 1; }
        if (p_Command == "write_Given_Trace_uint") { write_Given_Pattern_As_Number(p_File); return 1; }
        if (p_Command == "write_All_Traces") { gather_All_Traces(p_File); return 1; }
        if (p_Command == "write_All_Traces_uint") { gather_All_Traces_uint(p_File); return 1; }

        if (p_Command == "write_Node_Info") { gather_Given_Node(p_File); return 1; }
        if (p_Command == "write_Node_Info_As_Numbers") { gather_Given_Node_uint(p_File); return 1; }

        if (p_Command == "write_All_Nodes") { gather_All_Nodes(p_File); return 1; }
        if (p_Command == "write_All_Nodes_As_Numbers") { gather_All_Nodes_uint(p_File); return 1; }

        if (p_Command == "write_Output") { gather_Output(p_File); return 1; }
        if (p_Command == "write_Output_uint") { gather_Output_uint(p_File); return 1; }

        if (p_Command == "write_Treetop_Node") { gather_Treetop_Node(p_File); return 1; }
        if (p_Command == "write_Treetop_Node_Numbers") { gather_Treetop_Node_uint(p_File); return 1; }
        if (p_Command == "write_Treetop_NID_Only") { gather_Treetop_NID(p_File); return 1; }
        if (p_Command == "write_Treetop_NID") { get_Treetop_NID(p_File); return 1; }
        //if (p_Command == "write_Treetop_NID_To_Other_Input") { write_Treetop_NID_To_Other_Input(p_File); return 1; }

        //Console Output:
        if (p_Command == "output_Node_Raw") { output_Node_Raw(p_File); return 1; }
        if (p_Command == "output_Treetop_Node_Raw") { output_Treetop_Node_Raw(p_File); return 1; }
        if (p_Command == "output_Node_Network") { output_Node_Network(); return 1; }
        if (p_Command == "output_Node_Symbol") { output_Backpropagated_Symbol_NID(p_File); return 1; }
        if (p_Command == "output_Node_Symbol_uint") { output_Backpropagated_Symbol_NID_uint(p_File); return 1; }
        if (p_Command == "output_Symbols") { output_Backpropagated_Symbols(); return 1; }
        if (p_Command == "output_Input") { output_Input(p_File); return 1; }
        if (p_Command == "output_Input_uint") { output_Input_uint(p_File); return 1; }
        if (p_Command == "output_Output") { output_Output(p_File); return 1; }
        if (p_Command == "output_Output_uint") { output_Output_uint(p_File); return 1; }
        if (p_Command == "output_Scaffold_Char") { output_Scaffold_Char(p_File); return 1; }
        if (p_Command == "output_Scaffold_Symbols") { output_Scaffold_Symbols(p_File); return 1; }
        if (p_Command == "output_Scaffold_Symbols_uint") { output_Scaffold_Symbols_uint(p_File); return 1; }
        if (p_Command == "output_Node") { output_Node(p_File); return 1; }
        if (p_Command == "output_Constructs") { output_Constructs(); return 1; }

        //==--  Advanced Command List  --=//
        if (p_Command == "output_Node_Char") { output_Node_Char(p_File); return 1; }
        if (p_Command == "output_Scaffold") { output_Scaffold(p_File); return 1; }

        //Console Output:
        // 
        //Evaluation
        if (p_Command == "query_Spacial") { query_Spacial(p_File); return 1; }
        if (p_Command == "query_Given_Index") { query_Given_Index(p_File); return 1; }
        if (p_Command == "query_Given_Legs") { query_Given_Legs(p_File); return 1; }
        if (p_Command == "gather_Treetops") { gather_Treetops(p_File); return 1; }
        

        //Neural Network Construction and Initialization:
        if (p_Command == "set_State_Nodes_Index") { set_State_Nodes_Index(p_File); return 1; }

        //Node Manipulations
        if (p_Command == "set_Type") { set_Type(p_File); return 1; }
        if (p_Command == "add_Axon_Index") { add_Axon_Index(p_File); return 1; }
        if (p_Command == "set_Dendrites") { set_Dendrites(p_File); return 1; }
        if (p_Command == "does_Upper_Tier_Connection_Exist") { does_Upper_Tier_Connection_Exist(p_File); return 1; }
        if (p_Command == "does_Lower_Connection_Exist") { does_Lower_Connection_Exist(p_File); return 1; }
        if (p_Command == "bind_State") { bind_State(p_File); return 1; }

        //Network Manipulations
        if (p_Command == "get_Upper_Tier_Node") { get_Upper_Tier_Node(p_File); return 1; }
        if (p_Command == "does_State_Node_Exist") { does_State_Node_Exist(p_File); return 1; }
        if (p_Command == "get_State_Node") { get_State_Node(p_File); return 1; }
        if (p_Command == "does_Upper_Tier_Connection_Exist_Network") { does_Upper_Tier_Connection_Exist_Network(p_File); return 1; }
        if (p_Command == "new_Node") { new_Node(); return 1; }
        if (p_Command == "new_State_Node") { new_State_Node(p_File); return 1; }
        if (p_Command == "create_Connections") { create_Connections(p_File); return 1; }

        //Network Evaluation and Inference:
        if (p_Command == "submit_Set") { submit_Set(p_File); return 1; }

        //See if they submitted a command, these scripts are retrieved from the ./Scripts/ dir.
        std::string tmp_Command = "./Scripts/" + p_Command;
        interpret_File(tmp_Command);

        return 0;
    }


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
            if (flagValue != "")
            {
                // Check if the value is 1
                if (flagValue == "1")
                {
                    flagFile.close();
                    return 1;
                }
                else
                {
                    flagFile.close();
                }
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
    c_Construct_Text_Server(std::string p_Autoexec = "../autoexec.ssv")
    {
        Tick = 0;
        flg_Exit = false;

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

    //Hyperparams
    void set_Base_Charge(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Base_Charge |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        float tmp_Base_Charge = 0.0;

        *p_File >> tmp_Base_Charge;

        API.set_Base_Charge(tmp_Construct, tmp_Base_Charge);
    }

    void set_Modifier_Charge(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Modifier_Charge |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        float tmp_Modifier_Charge = 0.0;

        *p_File >> tmp_Modifier_Charge;

        API.set_Modifier_Charge(tmp_Construct, tmp_Modifier_Charge);
    }

    void set_Action_Potential_Threshold(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Action_Potential_Threshold |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        float tmp_Action_Potential_Threshold = 0.0;

        *p_File >> tmp_Action_Potential_Threshold;

        API.set_Action_Potential_Threshold(tmp_Construct, tmp_Action_Potential_Threshold);
    }

    void set_Charging_Tier(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Charging_Tier |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        int  tmp_Charging_Tier = 0;

        *p_File >> tmp_Charging_Tier;

        API.set_Charging_Tier(tmp_Construct, tmp_Charging_Tier);
    }

    //        ---==================---
    //       ---====================---
    //      ---======================---
    //     ---========================---
    //    ---==   NT4 Deep Control   ==---
    //     ---========================---
    //      ---======================---
    //       ---====================---
    //        ---==================---

    void create_Construct_Connection(std::ifstream* p_File)
    {
        std::cout << "\n --> create_Construct_Connection |";

        std::string tmp_Construct_From = "";

        *p_File >> tmp_Construct_From;

        std::cout << " " << tmp_Construct_From << " |";
        
        std::string tmp_Construct_To = "";

        *p_File >> tmp_Construct_To;

        std::cout << " " << tmp_Construct_To << " |";

        API.create_Construct_Connection(tmp_Construct_From, tmp_Construct_To);
    }

    void output_Construct_Connections(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Construct_Connections |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        API.output_Construct_Connections(tmp_Construct);
    }

    //Used for encoding.
    void round_Up_Input(std::ifstream* p_File)
    {
        std::cout << "\n --> round_Up_Input |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        API.round_Up_Input(tmp_Construct);
    }

    void pull_From_Lower_Connection(std::ifstream* p_File)
    {
        std::cout << "\n --> pull_From_Lower_Connection |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        int tmp_Lower_Connection = 0;

        *p_File >> tmp_Lower_Connection;

        API.pull_From_Lower_Connection(tmp_Construct, tmp_Lower_Connection);
    }
    

    void pull_Chrono_From_Lower_Connection(std::ifstream* p_File)
    {
        std::cout << "\n --> pull_Chrono_From_Lower_Connection |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        API.pull_Chrono_From_Lower_Connection(tmp_Construct);
    }

    //Used for eval, pulls the output from lower to upper.
    void pull_From_Lower_Connections(std::ifstream* p_File)
    {
        std::cout << "\n --> pull_From_Lower_Connections |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        API.pull_From_Lower_Connections(tmp_Construct);
    }

    void pull_From_Upper_Index(std::ifstream* p_File)
    {
        std::cout << "\n --> pull_From_Upper_Index |";

        std::string tmp_Construct_To = "";

        *p_File >> tmp_Construct_To;

        std::string tmp_Construct_From = "";

        *p_File >> tmp_Construct_From;

        int tmp_Index = 0;

        *p_File >> tmp_Index;

        API.pull_From_Upper_Index(tmp_Construct_To, tmp_Construct_From, tmp_Index);
    }

    //      ---==========---
    //     ---============---
    //    ---==   Node   ==--- 
    //     ---============---
    //      ---==========---

    /** Sets the type of a given node.

        set_Type NID TYPE
    \param uint64_t NID The Node ID (NID) of the node being manipulated.
    \param uint64_t TYPE The type you are assigning the node to: (0: State), (1: Branch), (2: Treetop), (3: State/Treetop)
    \retval None This function doesn't return any values.

    This type is used for determining how to handle the node. You may want to set one to a treetop that way you can then write a charging function that has a different behavior for treetop nodes.

    Example Usage:

    Setting the type of node 66 to the value of 2 flagging it as a treetop node. Before and after setting the type we use output_Node_Raw to view the results. This assumes node 66 has two dendrites { 7, 14 }.

        output_Node_Raw 66
        set_Type 66 2
        output_Node_Raw 66

    Output:

        Node_ID (NID) 66 RC: 1 Type: 0  State { 0 } --- Dendrites:  [7]  [14]  --- Axon_Hillock_Count: 0
        Node_ID (NID) 66 RC: 1 Type: 2  State { 0 } --- Dendrites:  [7]  [14]  --- Axon_Hillock_Count: 0

    Error Handling:

    - No error handling is implemented in this function. 

    Additional Notes:

    - Calling this function with an invalid node will cause a crash. 
    - You can set the type to any value you want, this allows for extending functionality, but can cause a crash if you set it to an invalid type and the node is mishandled.
    */
    void set_Type(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Type NID TYPE |";

        int tmp_NID = 0;

        *p_File >> tmp_NID;

        int tmp_Type = 0;

        *p_File >> tmp_Type;

        API.set_Type(tmp_NID, tmp_Type);

        //std::cout << " [|x|]";
    }



    /** Adds a given NID to an Axon on the given Hillock. Note this is selecting a hillock not assigning an axonal ID, if the index doesn't exist then exist it with resize_Axon_Hillocks()

        add_axon_index NID Axon_NID HILLOCK_INDEX
    \param uint64_t NID The Node ID (NID) of the node being manipulated.
    \param uint64_t AXON_NID The NID of the node that the axon will reference.
    \param int INDEX The hillock the axon is put onto, not the axon ID.
    \retval None This function doesn't return any values.

    The nodes keep track of which dendrite their upper connections are connected to by encoding them into respective "axon hillocks". Basically we take whichever dendrite index the connection is connected to and create an axonic group that represents those connections to legs of that index. This allows for advanced search of the networks including time-series and more.

    Example Usage:

    Adding an axonic connection to node 42 on axon hillock 5 of node 16 to represent that it is Dendrite[5] on the node above. We will use output_Node_Raw after setting the axon to show the result. This assumes no previous axonic connections or dendrites and it is a quanta bound state node with the state set to the uint64_t value 99.

        output_Node_Raw 16
        add_Axon_Index 16 42 5
        output_Node_Raw 16

    Output:

        Node_ID (NID) 16 RC: 1 Type: 0  State { 99 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 16 RC: 1 Type: 0  State { 99 } --- Dendrites:  --- Axon_Hillock_Count: 6  Axons[0] <0>:  Axons[1] <0>:  Axons[2] <0>:  Axons[3] <0>:  Axons[4] <0>:  Axons[5] <1>: [0] 42

    Error Handling:

    - No error handling is implemented in this function. 

    Additional Notes:

    - Calling this function with an invalid node will cause a crash. 
    - Invalid axon IDs will not cause an immediate crash, but any function which touches the axons will crash when attemting to reference the invalid axon NIDs.
    */
    void add_Axon_Index(std::ifstream* p_File)
    {
        std::cout << "\n --> add_axon_index NID Axon_NID HILLOCK_INDEX |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        uint64_t tmp_Axon_NID = 0;

        *p_File >> tmp_Axon_NID;

        std::cout << " " << tmp_Axon_NID << " |";

        int tmp_Index = 0;

        *p_File >> tmp_Index;

        std::cout << " " << tmp_Index << " |";

        API.add_Axon_Index(tmp_NID, tmp_Axon_NID, tmp_Index);

        //std::cout << " [|x|]";
    }



    /** Sets the dendrites of the node.

        set_Dendrites NID COUNT DENDRITE_IDS 
    \param uint64_t NID The Node ID of the node being manipulated.
    \param int COUNT The number of dendrites in the array DENDRITE_IDS 
    \param uint64_t DENDRITE_IDS  An array of NIDs that represent the dendritic connections.
    \retval None This function doesn't return any values.

    The choice of encoding method will determine dendrite count. This is left with a dynamic leg count though so that bespoke networks can be created with ease. It is dangerous, but that is up to the user to handle.

    Example Usage:

    Setting the dendrites of imaginary node 43 to NIDs 2 & 3 representing the connections to "lower" tier nodes making up the sub-symbols of the current node's represented pattern. Before and after setting them we output the node. This assumes no axonic connections.
        
        output_Node_Raw 43
        set_Dendrites 43 2 2 3
        output_Node_Raw 43

    Output:

        Node_ID (NID) 43 RC: 1 Type: 2  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 43 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 2 [1] 3 --- Axon_Hillock_Count: 0

    Error Handling:

    - No error handling is implemented in this function. 

    Additional Notes:

    - This assumes the node has no dendrites yet, if it does you be dangling and jangling.
    - Calling this function with an invalid node will cause a crash. 
    - Calling it and assigning dendrites invalid NID IDs will cause a crash when bp_O, create_Connection, does_Lower_Ter_Connection_Exist, or any other operation which touches the dendrites.
    */
    void set_Dendrites(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Dendrites NID COUNT DENDRITES  |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t * tmp_Dendrites = NULL;
        tmp_Dendrites = new uint64_t [tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Dendrites[cou_Index];

            std::cout << " " << tmp_Dendrites[cou_Index] << " |";
        }

        API.set_Dendrites(tmp_NID, tmp_Count, tmp_Dendrites);

        if (tmp_Dendrites != NULL) { delete [] tmp_Dendrites; tmp_Dendrites = NULL; }

        //std::cout << " [|x|]";
    }


    /** Searches the axons to see if an upper tier connection exists, if found returns the NID, if not returns 0.

        does_Upper_Tier_Connection_Exist NODE_COUNT NODES 
    \param int NODE_COUNT The number of nodes to use in the check for an upper tier node symbol.
    \param uint64_t NODES  The array of NIDs to look for.
    \retval uint64_t NID This function return the NID of a node matching the description, returns 0 if none are found.

    This is used during encoding and performing a query. It traverses the connections in the given NID's first axon hillock. For each of these connections it does a does_Lower_Connection_Exist to see if that node matches. By searching the first legs we keep the search limited to only those with the possiblity to match the search string of connections. We know this is the case because if the first node in a string of nodes has an upper tier connection then that gives us a connection to a node that at least shares index[0] with our current lower-tier node and by checking every other dendrite index of the upper tier node against the respective index in the passed node array we can know for sure if our current pattern exists yet. After checking all of the upper tier connections on axon hillock 0 if we come up with no confirmed connections we know that the current arrangement of nodes hasn't been encoded into a higher tier node symbol yet.

    Example Usage:

    We will use the example containing a set of nodes { 5 6 7 }, node 5 has 2 axonic connections with the second having the correct configuration of legs meaning we found it. This second node has the NID 75, we know node 75 has the correct connection so we output it manually to show the node data.

        output_Raw_Node 75
        does_Upper_Tier_Connection_Exist 3 5 6 7

    Output:

        Node_ID (NID) 75 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 5 [1] 6 [2] 7 --- Axon_Hillock_Count: 0

    Contents of the RETURN_FILE:
    
        75

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Calling this function with an invalid node will cause a crash.
    */
    uint64_t does_Upper_Tier_Connection_Exist(std::ifstream* p_File)
    {
        std::cout << "\n --> does_Upper_Tier_Connection_Exist NODE_COUNT NODES  |";

        uint64_t tmp_Return_NID = 0;

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t* tmp_Nodes = NULL;
        tmp_Nodes = new uint64_t[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Nodes[cou_Index];

            std::cout << " " << tmp_Nodes[cou_Index] << " |";
        }

        tmp_Return_NID = API.does_Upper_Tier_Connection_Exist(tmp_Count, tmp_Nodes);

        if (tmp_Nodes != NULL) { delete [] tmp_Nodes; tmp_Nodes = NULL; }

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** Checks if the given node has dendrites that match the given nodes in the given order.

        does_Lower_Connection_Exist NID NODE_COUNT NODES 
    \param uint64_t NID The Node ID of the node being manipulated.
    \param uint64_t NODE_COUNT The number of nodes to use in the check for an upper tier node symbol.
    \param uint64_t NODES  The array of NIDs to look for.
    \retval bool This function returns a 1 (true) or 0 (false) depending on whether the dendrites match the given nodes or not.

    This takes a given node and a set of node IDs (NIDs) to check. It checks each dendrite leg against the given node list to see if they all match or not. If one is found that doesn't match it return 0 immediately. Used during encoding to find out whether a compound higher tier symbol has been encoded yet.

    Example Usage:

    We will check node 75 which has the dendrites { 5, 6, 7 } to see if it matches the given dendrite set { 5, 6, 8 }, which we know it does not. Then we will output a newline in the output file and check node 76 which in this example has the dendrites { 5, 6, 8 } which match the given set outputting 1 to the RETURN_FILE. Before checking each node we'll output it to show the dendrites so you can see them yourself.

        output_Node_Raw 75
        does_Lower_Connection_Exist 75 3 5 6 8
        newline returns
        output_Node_Raw 76
        does_Lower_Connection_Exist 76 3 5 6 8

    Output:
    
        Node_ID (NID) 75 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 5 [1] 6 [2] 7 --- Axon_Hillock_Count: 0
        Node_ID (NID) 76 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 5 [1] 6 [2] 8 --- Axon_Hillock_Count: 0

    Contents of the RETURN_FILE:

        0
        1

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Calling this function with an invalid node will cause a crash.
    - If the given node count exceeds the dendrite count you will cause it to crash. This is due to the function assuming the count will not exceed the number of defined dendrites and iterates trustfully through it. This is by design as this function will be called many often and is a potential bottleneck.
    */
    bool does_Lower_Connection_Exist(std::ifstream* p_File)
    {
        std::cout << "\n --> does_Lower_Connection_Exist NID NODE_COUNT NODES  |";

        bool tmp_Return_Result = 0;

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t* tmp_Nodes = NULL;
        tmp_Nodes = new uint64_t[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Nodes[cou_Index];

            std::cout << " " << tmp_Nodes[cou_Index] << " |";
        }

        tmp_Return_Result = API.does_Lower_Connection_Exist(tmp_NID, tmp_Count, tmp_Nodes);

        if (tmp_Nodes != NULL) { delete [] tmp_Nodes; tmp_Nodes = NULL; }

        write_to_output(RETURN_FILE, tmp_Return_Result);

        return tmp_Return_Result;

        //std::cout << " [|x|]";
    }



    /** Binds a node to a quanta of data, the state of the input, as uint64_t.

        bind_State NID STATE
    \param uint64_t NID The Node ID of the node being manipulated.
    \param uint64_t NID The state of the input, the current quanta, or element, of information being encoded.
    \retval None This function doesn't return any values.

    At the interface between the networks internal node structure and the exterior information space exists the state tier, this is the interface where outside information binds to nodes to allow for communication coherently across the internal/external space. Outside you have the information to be encoded, this information is composed of differentiated information, the smallest piece of coherent information that the abstractions are built upon is the quanta. This quanta is a pixel, a character, an integer, a scalar value. A quantifiable and discrete piece of information which forms the basis of the patterns of information you will be encoding/decoding from the neural matrix. This doesn't have to be low level abstraction wise, only that is has a hash or scalar abstraction that can be cast to uint64. It could represent the hash of a full 3D body scan as easily as a scalar value from a thermal sensor.
    
    This function binds the state to the node to form the interface, an absolutely crucial function to the distributed symbol neural networks.

    Example Usage:

    We will bind the uint state 1234321 to node 10, outputting the node before and after the operation using 'output_Node_Raw 10'.

        output_Node_Raw 10
        bind_State 10 1234321
        output_Node_Raw 10

    Output:

        Node_ID (NID) 10 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 10 RC: 1 Type: 0  State { 1234321 } --- Dendrites:  --- Axon_Hillock_Count: 0

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Calling this function with an invalid node will cause a crash.
    */
    void bind_State(std::ifstream* p_File)
    {
        std::cout << "\n __COMMAND__| bind_State NID STATE |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";
        
        uint64_t tmp_State = 0;

        *p_File >> tmp_State;

        std::cout << " " << tmp_State << " |";

        API.bind_State(tmp_NID, tmp_State);

        //std::cout << " [|x|]";
    }



    /** Initiates a backpropagation (feedbackward info cascade ! weight adjustment) that outputs the pattern represented by this node.

        bp_O NID
    \param uint64_t NID The Node ID of the node who's information symbol is being output.
    \retval None This function doesn't return any values.

    The patterns that are encoded, the traces, are encoded in a deterministic and hierarchical manner. This allows us the oppurtunity to craft algorithms to "walk" the structure down and retrieve the original input pattern stored in the relationships of the nodes on the various tiers, or layers, or abstraction.

    This function outputs the full symbol represented by the node given. It walks the tree to output the pattern perfectly as it was originally encoded.

    Example Usage:

    In this example we have encoded the pattern "64 128 256" into a many to one construct. In this example the nodes 1-9 have already been created so our first node will be NID 10. This results in 4 nodes created, 3 state tier nodes { 10, 11, 12 } and 1 upper tier treetop node { 13 }. We will output all the nodes, then demonstrate the bp_O.

        output_Node_Raw 10
        output_Node_Raw 11
        output_Node_Raw 12
        output_Node_Raw 13
        bp_O 13

    Output:

        Node_ID (NID) 10 RC: 1 Type: 0  State { 64 } --- Dendrites:  --- Axon_Hillock_Count: 1  Axons[0] <1>: 13
        Node_ID (NID) 11 RC: 1 Type: 0  State { 128 } --- Dendrites:  --- Axon_Hillock_Count: 2  Axons[0] <0>:  Axons[1] <1>: 13
        Node_ID (NID) 12 RC: 1 Type: 0  State { 256 } --- Dendrites:  --- Axon_Hillock_Count: 3  Axons[0] <0>:  Axons[1] <0>:  Axons[2] <1>: 13
        Node_ID (NID) 13 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 10 [1] 11 [2] 12 --- Axon_Hillock_Count: 0
        <=- 13 * [double(64), 64]  [double(128), 128]  [double(256), 256] -=>

    - Note that the 'double(n)' in the real output is the actual cast value and not the typecast definition.

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Calling this function with an invalid node will cause a crash.
    - If you crash when calling this and the node is valid then check your dendrites.
    */
    void bp_O(std::ifstream* p_File)
    {
        std::cout << "\n __COMMAND__| bp_O NID |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.bp_O(tmp_NID);

        //std::cout << " [|x|]";
    }



    /** Outputs the ugly raw info dump for the node.

        output_Node_Raw NID
    \param uint64_t NID The Node ID of the node who's information symbol is being output.
    \retval None This function doesn't return any values.

    This is pretty basic, outputs the node.

    Here is the breakdown of the components output:

    -Entire output node raw dump:

        Node_ID (NID): %NID% RC: %RC% Type: %Type% State { %State% } --- Dendrites: [%Dendrite_ID%] %Dendrites[%Dendrite_ID%]% --- Axon_Hillock_Count: %Axon_Hillock_Count% Axons[%Axon_Hillock_ID%] <%Axon_Count_For_Hillock%>: [%Axon_ID%] Axons[%Axon_Hillock_ID%][%Axon_ID%]

    - The NID is the node ID within the network of nodes.

        Node_ID (NID): %NID% 

    - The RC is the reinforcement counter, the reinforcement value, the neuroplasticity analogue, the post-synaptic-cleft density approximation.

        RC: %RC% 

    - The type of node, see set_Node_Type for more information.

        Type: %Type% 

    - The pattern of input quanta that the node represents, the hierarchically encoded sensory trace, the abstract symbol built from the recomposition of the input states into the complete encoded pattern.

        State { %State% } 

    - The "lower" connections to nodes that form sub-symbols of the current node's symbol.

        Dendrites: [%Dendrite_ID%] %Dendrites[%Dendrite_ID%]% 

    - The number of axon hillocks on this node, the number of axonal hillocks is determined by the position of upper tier connections. If an upper tier axonic connection makes contact with dendritic index 4 then this lower node will have 5 axon hillocks so that number 4 can hold this connection preserving the information of the upper tier connection index without having to query the other node to get the respective position. This is used during the function which check for upper tier nodes and during some charging methodologies.

        Axon_Hillock_Count: %Axon_Hillock_Count% 
        
    - If the node contains axonic connections they will be iteratively printed here. For each hillock the number of axons on it will be printed followed by the individual axonic connections.

        Axons[%Axon_Hillock_ID%] <%Axon_Count_For_Hillock%>: [%Axon_ID%] Axons[%Axon_Hillock_ID%][%Axon_ID%]


    Example Usage:

    In this example we have encoded the pattern "64 128 256" into a many to one construct. In this example the nodes 1-9 have already been created so our first node will be NID 10. This results in 4 nodes created, 3 state tier nodes { 10, 11, 12 } and 1 upper tier treetop node { 13 }. We will output all the nodes using this function to show state, treetop, dendritically connected, and axonically connected nodes..

        output_Node_Raw 10
        output_Node_Raw 11
        output_Node_Raw 12
        output_Node_Raw 13

    Output:

        Node_ID (NID) 10 RC: 1 Type: 0  State { 64 } --- Dendrites:  --- Axon_Hillock_Count: 1  Axons[0] <1>: 13
        Node_ID (NID) 11 RC: 1 Type: 0  State { 128 } --- Dendrites:  --- Axon_Hillock_Count: 2  Axons[0] <0>:  Axons[1] <1>: 13
        Node_ID (NID) 12 RC: 1 Type: 0  State { 256 } --- Dendrites:  --- Axon_Hillock_Count: 3  Axons[0] <0>:  Axons[1] <0>:  Axons[2] <1>: 13
        Node_ID (NID) 13 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 10 [1] 11 [2] 12 --- Axon_Hillock_Count: 0

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Calling this function with an invalid node will cause a crash.
    */
    void output_Node_Raw(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Node_Raw NID |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.output_Node_Raw(tmp_NID);

        //std::cout << " [|x|]";
    }



    void output_Treetop_Node_Raw(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Treetop_Node_Raw CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.output_Treetop_Node_Raw(tmp_Construct);

        //std::cout << " [|x|]";
    }



    /** Casts the node address to char() and outputs it.

        output_Node_Char NID
    \param uint64_t NID The Node ID of the node who's NID is being output as a char.
    \retval None This function doesn't return any values.

    This function may seem pointless at first, but it is used in conjunction with the CAN scaffolds and others to output a compact visualizatin of a series of nodes by their NID. Though there is overlap when typecasting NIDs the variation is enough that the human can see differences in different permutations of encoded input.

    Example Usage:
    
    In this example we have encoded the pattern "64 128 256" into a many to one construct. In this example the nodes 0 - 175 have already been created so our first node will be NID 176. The node bound to state 128 already exists with a NID 65. This results in 3 nodes created, 2 state tier nodes { 176, 177 } and 1 upper tier treetop node { 178 }. We will output all the nodes using this function to show a small example of the potential output used to convey variance int he NID structure at a glance.

        output_Node_Char 176
        output_Node_Char 65
        output_Node_Char 177
        output_Node_Char 178

    Output:

        ░A▒▓

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Calling this function with an invalid node will cause a crash.
    */
    void output_Node_Char(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Node_Char NID |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.output_Node_Char(tmp_NID);

        //std::cout << " [|x|]";
    }





    //    ---==================---
    //   ---====================---
    //  ---==   Node_Network   ==---
    //   ---====================---
    //    ---==================---
  
	/** Used to differentiate a new construct, allocates a state tree to handle a state-node space.


    /** Creates a new node in the shared node network.

        new_Node
    \retval uint64_t This returns the NID of the newly created nodes.

    When the node is created it is stored in a linked list. This new node is then added to the node registration fractal tree. This allows for storing the node network in a linked list for when we want to iterate over them, and for searching with a predictable search time for nodes in the linked list no matter its length.

    Example Usage:

    In this example we register a new node using new_Node and then check the output file for the NID of the newly created node. Currently the node network is at 249 nodes meaning the new one will have a NID of 250.

        new_Node

    Output:

    Contents of the RETURN_FILE:

        250

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    uint64_t new_Node()
    {
        std::cout << "\n --> new_Node |";

        uint64_t tmp_Return_NID = API.new_Node();

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** Creates a new node, binds it to a state, sets the type to 0 or 3 depending, and adds the node to the state tree with the state registered. 

        new_State_Node CONSTRUCT_ID STATE
    \param uint64_t CONSTRUCT_ID This is the Construct to use when creating the state node.
    \param uint64_t STATE The state to bind to the node.
    \retval uint64_t Returns the NID of the newly minted state node.

    By registering the node with the fractal state tree we can have predictable search times no matter how big the tree gets. 

    Example Usage:

    In this example we have 3 constructs registered and we use 'new_State_Node 2 121' to create a state node for construct [2], in this example construct [2] currently has 28 nodes registered making the return value 29 as that is the new nodes NID.

        new_State_Node 2 121

    Output:

    Contents of the RETURN_FILE:

        29

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Assumes the construct is already registered with a valid index.
    */
    uint64_t new_State_Node(std::ifstream* p_File)
    {
        std::cout << "\n --> new_State_Node CONSTRUCT_ID STATE |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";
        
        uint64_t tmp_State = 0;

        *p_File >> tmp_State;

        std::cout << " " << tmp_State << " |";

        uint64_t tmp_Return_NID = API.new_State_Node(tmp_Construct, tmp_State);

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** Creates a connection between nodes.

        create_Connections TO_NID COUNT FROM_NIDS 
    \param uint64_t TO_NID This is the node that all the lower nodes connect to, the one that receives the dendritic connections.
    \param uint64_t NODE_COUNT The number of nodes to connect to the upper tier node.
    \param uint64_t NODES  The array of node NIDs that are the lower nodes, they receive the axonic connections.
    \retval None This function doesn't return any values.

    p_To forms dendritic connections to p_From, and on p_From you have the axonic connections. This function uses set_Dendrites on the upper tier node and sets the axon for the lower nodes to create the two way connection. The doubly linked tiered nodes.

    Example Usage:

    In this example we have an upper tier node we created with the NID 100, and 5 lower nodes with NIDs { 10, 20, 30, 40, 50 }. We assume no nodes have any connections as of yet. We will be outputting the nodes before and after.

        output_Node_Raw 10
        output_Node_Raw 20
        output_Node_Raw 30
        output_Node_Raw 40 
        output_Node_Raw 50
        output_Node_Raw 100

        create_Connections 100 5 10 20 30 40 50

        output_Node_Raw 10
        output_Node_Raw 20
        output_Node_Raw 30
        output_Node_Raw 40
        output_Node_Raw 50
        output_Node_Raw 100

    Output:

        Node_ID (NID) 10 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 20 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 30 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 40 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 50 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
        Node_ID (NID) 100 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0

        Node_ID (NID) 10 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 1  Axons[0] <1>: 100
        Node_ID (NID) 20 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 2  Axons[0] <0>:  Axons[1] <1>: 100
        Node_ID (NID) 30 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 3  Axons[0] <0>:  Axons[1] <0>:  Axons[2] <1>: 100
        Node_ID (NID) 40 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 3  Axons[0] <0>:  Axons[1] <0>:  Axons[2] <0>:   Axons[2] <1>: 100
        Node_ID (NID) 50 RC: 1 Type: 0  State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 3  Axons[0] <0>:  Axons[1] <0>:  Axons[2] <0>:   Axons[2] <0>:   Axons[2] <1>: 100
        Node_ID (NID) 100 RC: 1 Type: 0  State { 0 } --- Dendrites: [0] 10 [1] 20 [2] 30 [3] 40 [4] 50  --- Axon_Hillock_Count: 0
        

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - This does nothing for setting or binding states, or setting node types, this function is purely to create these connections.
    - Passing invalid nodes will cause it to crash when it tries to call the NULL object to create either dendrite or axon connections.
    */
    void create_Connections(std::ifstream* p_File)
    {
        std::cout << "\n --> create_Connections TO_NID COUNT FROM_NIDS  |";

        uint64_t tmp_To_NID = 0;

        *p_File >> tmp_To_NID;

        std::cout << " " << tmp_To_NID << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t* tmp_Nodes = NULL;
        tmp_Nodes = new uint64_t[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Nodes[cou_Index];

            std::cout << " " << tmp_Nodes[cou_Index] << " |";
        }

        API.create_Connections(tmp_To_NID, tmp_Count, tmp_Nodes);

        if (tmp_Nodes != NULL) { delete [] tmp_Nodes; tmp_Nodes = NULL; }

        //std::cout << " [|x|]";
    }


    /** Checks if an upper tier node exists, functionally the same as does_Upper_Tier_Connection_Exist on the node, but this includes error handling.

        does_Upper_Tier_Connection_Exist_Network NODE_COUNT NODES 
    \param int NODE_COUNT The number of nodes to use in the check for an upper tier node symbol.
    \param uint64_t NODES  The array of NIDs to look for.
    \retval uint64_t NID This function return the NID of a node matching the description, returns 0 if none are found.

    This is used during encoding and performing a query. It traverses the connections in the given NID's first axon hillock. For each of these connections it does a does_Lower_Connection_Exist to see if that node matches. By searching the first legs we keep the search limited to only those with the possiblity to match the search string of connections. We know this is the case because if the first node in a string of nodes has an upper tier connection then that gives us a connection to a node that at least shares index[0] with our current lower-tier node and by checking every other dendrite index of the upper tier node against the respective index in the passed node array we can know for sure if our current pattern exists yet. After checking all of the upper tier connections on axon hillock 0 if we come up with no confirmed connections we know that the current arrangement of nodes hasn't been encoded into a higher tier node symbol yet.

    Example Usage:

    We will use the example containing a set of nodes { 5 6 7 }, node 5 has 2 axonic connections with the second having the correct configuration of legs meaning we found it. This second node has the NID 75, we know node 75 has the correct connection so we output it manually to show the node data.

        output_Raw_Node 75
        does_Upper_Tier_Connection_Exist_Network 3 5 6 7

    Output:

        Node_ID (NID) 75 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 5 [1] 6 [2] 7 --- Axon_Hillock_Count: 0

    Contents of the RETURN_FILE:
    
        75

    Error Handling:

    - This function makes sure that the first node on the submitted node set is not NULL before calling does_Upper_Tier_Connection_Exist. This command is safer than the direct node 'does_Upper_Tier_Connection_Exist'.

    Additional Notes:

    - None
    */
    uint64_t does_Upper_Tier_Connection_Exist_Network(std::ifstream* p_File)
    {
        std::cout << "\n --> does_Upper_Tier_Connection_Exist_Network NODE_COUNT NODES  |";

        uint64_t tmp_Return_NID = 0;

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t* tmp_Nodes = NULL;
        tmp_Nodes = new uint64_t[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Nodes[cou_Index];

            std::cout << " " << tmp_Nodes[cou_Index] << " |";
        }

        tmp_Return_NID = API.does_Upper_Tier_Connection_Exist_Network(tmp_Count, tmp_Nodes);

        if (tmp_Nodes != NULL) { delete [] tmp_Nodes; tmp_Nodes = NULL; }

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** Gets an upper tier node based on the given legs, will create it if not found.

        get_Upper_Tier_Node NODE_COUNT NODES 
    \param int NODE_COUNT The number of nodes to use in the check for an upper tier node symbol.
    \param uint64_t NODES  The array of NIDs to look for.
    \retval uint64_t NID The node that represents the symbol containing the lower nodes passed to this function. Passes the NID back whether found or newly minted.

    This function uses 'does_Upper_Tier_Node_Exist' to find if the node exists or not. If the node is not found then this function creates the node, assigns the connections, and passes the prepared NID back.

    Example Usage:

    We will use the example containing a set of nodes { 5 6 7 }, in our example node 75 matches this configuration. We output it manually to show the node data proving this and we show the returned NID in the returned data file.

        output_Raw_Node 75
        get_Upper_Tier_Node 3 5 6 7

    Output:

        Node_ID (NID) 75 RC: 1 Type: 2  State { 0 } --- Dendrites:  [0] 5 [1] 6 [2] 7 --- Axon_Hillock_Count: 0

    Contents of the RETURN_FILE:

        75

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - If you get a crash when calling this then it is likely one or more of your submitted NIDs are not valid.
    */
    uint64_t get_Upper_Tier_Node(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Upper_Tier_Node NODE_COUNT NODES  |";

        uint64_t tmp_Return_NID = 0;

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t* tmp_Nodes = NULL;
        tmp_Nodes = new uint64_t[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Nodes[cou_Index];

            std::cout << " " << tmp_Nodes[cou_Index] << " |";
        }

        tmp_Return_NID = API.get_Upper_Tier_Node(tmp_Count, tmp_Nodes);

        if (tmp_Nodes != NULL) { delete [] tmp_Nodes; tmp_Nodes = NULL; }

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** If a state node exists in the given construct index then return it, otherwise return 0.

        does_State_Node_Exist CONSTRUCT_ID STATE
    \param int CONSTRUCT_ID The construct that encapsulates the state space to search.
    \retval uint64_t NID The node that represents the state passed to this function. Passes the NID back if found, if not found it passes back 0.

    Checks the state tree of a given construct to see if the state is registered and bound to a node within that state space.

    Example Usage:

    We have a state node registered to construct 3 with the state 123. This state node is NID 50. We run 'does_State_Node_Exist 2 123' which returns the NID 50 as it found the node bound to the state '123' registered within construct 2's fractal state tree.

        does_State_Node_Exist 2 123

    Output:

    Contents of the RETURN_FILE:

        50

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - This assumes the [CONSTRUCT_ID] is valid
    */
    uint64_t does_State_Node_Exist(std::ifstream* p_File)
    {
        std::cout << "\n --> does_State_Node_Exist CONSTRUCT_ID STATE |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        uint64_t tmp_State = 0;

        *p_File >> tmp_State;

        std::cout << " " << tmp_State << " |";

        uint64_t tmp_Return_NID = API.does_State_Node_Exist(tmp_Construct, tmp_State);

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** Checks to see if a node in the given Construct is bound to the given state, if not the node is created & the NID returned, if found the NID is returned..

        get_State_Node CONSTRUCT_ID STATE
    \param int CONSTRUCT_ID The construct that encapsulates the state space to search.
    \param uint64_t NID The state of the input, the current quanta, or element, of information being encoded.
    \retval uint64_t NID The node that represents the state passed to this function. Passes the NID back whether found or newly minted.

    Describe behavior and side-effects. What it basically does and if it touches any globals.

    Example Usage:

    Here we have a state that hasn't been encoded yet, this '999'. We run 'get_State_Node 2 999' which returns the NID of the newly minted and state bound node now stored and registered with construct 2's state fractal tree. In this example the NID of the new node is 150. We see this NID in the return file.

        does_State_Node_Exist 2 999

    Output:

    Contents of the RETURN_FILE:

        150

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - This assumes the [CONSTRUCT_ID] is valid
    */
    uint64_t get_State_Node(std::ifstream* p_File)
    {
        std::cout << "\n --> get_State_Node CONSTRUCT_ID STATE |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        uint64_t tmp_State = 0;

        *p_File >> tmp_State;

        std::cout << " " << tmp_State << " |";

        uint64_t tmp_Return_NID = API.get_State_Node(tmp_Construct, tmp_State);

        write_to_output(RETURN_FILE, tmp_Return_NID);

        return tmp_Return_NID;

        //std::cout << " [|x|]";
    }


    /** Iterates through every node and outputs their bp_O()

        output_BP
    \retval None This function doesn't return any values.

    This is for outputting the entire network to the console.

    Example Usage:

    in this example we have a single trace encoded which is composed of 5 integers, { 11, 22, 33, 44, 55 }. This results in 6 nodes which we will see in the output. 5 state tier nodes, one for each input integer, and one higher tier node. Remember, NID 0 exists, but it is the NULL node not to be used.

        output_BP

    Output:

        <=- 0 [0] -=>
        <=- 1 [11] -=>
        <=- 2 [22] -=>
        <=- 3 [33] -=>
        <=- 4 [44] -=>
        <=- 5 [55] -=>
        <=- 6 * [11]  [22]  [33]  [44]  [55] -=>

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Backpropagated_Symbols()
    {
        std::cout << "\n --> output_Backpropagated_Symbols |";

        API.output_Backpropagated_Symbols(2);

        //std::cout << " [|x|]";
    }


    /** Finds given NID and outputs the bp_O().

        output_BP_NID NID
    \param uint64_t NID The Node ID (NID) of the node being manipulated.
    \retval None This function doesn't return any values.

    Uses the fractal NID tree to search the node linked list for the given NID. Once found it uses the ->bp_O() to output the pattern the node represents.

    Example Usage:

    Here NID 55 represents the compound symbol '32 64 128 64 32'.

        bp_NID 5

    Output:

        <=- 55 * [32]  [64]  [128]  [64]  [32] -=>

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Backpropagated_Symbol_NID(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Backpropagated_Symbol_NID   NID |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.output_Backpropagated_Symbol_NID(tmp_NID);

        //std::cout << " [|x|]";
    }


    void output_Backpropagated_Symbol_NID_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Backpropagated_Symbol_NID_uint   NID |";

        uint64_t tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.output_Backpropagated_Symbol_NID_uint(tmp_NID);

        //std::cout << " [|x|]";
    }

    //    ---=========---
    //   ---===========---
    //  ---==   CAN   ==---
    //   ---===========---
    //    ---=========---

    //   ---=============================================---
    //  ---==   Encoding and querying the constructs.   ==---
    //   ---=============================================---

	/** Encodes the current input of the given Construct.

        encode CONSTRUCT_ID
    \param int CONSTRUCT_ID The construct that encapsulates the state space to use when encoding.
    \retval None This function doesn't return any values.

    This encodes the p_Input data, if the nodes aren't found they are created, used for training. How it does this is determined by what type of construct it is. Different constructs can have different structures depending on whether they are many to one, 2d pyramidal, 3d pyramidal, stiched based, etc.

    To encode a construct you must first have input loaded into it. Once the input is loaded then calling this will encode the input pattern into a construct. The way to use this is to first get input to the construct, then you tell it to encode the input, then you can use gather, output, and other functions to access the results.:

         Get Input: set_Input | set_Input_uint | load_Input
         Do something with the Input: encode
         Get your Output: gather_X | output_X 

    Example Usage:

    - First we create a construct of the type Many_To_One named ExaCon for Example Construct.
    - Then we use set_Input_uint to load the uint array { 10, 15, 20 } into ExaCon's input.
    - After the input is set we call encode on ExaCon, which is construct ID [0].
    - We then use output_Scaffold to view the encoded trace.
    - This example is a Many_To_One, which means all state nodes (Tier[0]) are connected to a single upper tier (Tier[1]) treetop node.
        
        register_Construct Many_To_One ExaCon
        set_Input_uint 0 5 3 10 15 20
        encode 0
        output_Scaffold 0
        output_Node_Network
        gather_All_Nodes 0

    Output:

         __COMMAND__| output_Scaffold CONSTRUCT_ID |
          --==   CAN_Scaffold   ==--
         <- Tier[0] ->
           [0]  <> 1
           [1]  <> 2
           [2]  <> 3
         <- Tier[1] ->
           [0]  <> 4

         __COMMAND__| output_node_network |
         Node_ID (NID): 0  RC: 1 Type: 0 State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
         Node_ID (NID): 1  RC: 2 Type: 0 State { 10 } --- Dendrites:  --- Axon_Hillock_Count: 1 Axons[0] <1>:  [0] 4
         Node_ID (NID): 2  RC: 2 Type: 0 State { 15 } --- Dendrites:  --- Axon_Hillock_Count: 2 Axons[0] <0>:  Axons[1] <1>:  [0] 4
         Node_ID (NID): 3  RC: 2 Type: 0 State { 20 } --- Dendrites:  --- Axon_Hillock_Count: 3 Axons[0] <0>:  Axons[1] <0>:  Axons[2] <1>:  [0] 4
         Node_ID (NID): 4  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 1 [1] 2 [2] 3 --- Axon_Hillock_Count: 0

    - Here is another example, except this time we're encoding the phrase "the quick brown fox jumped over the lazy dog" to show language being encoded.

        register_Construct Many_To_One ExaCon
        set_Input 0 the quick brown fox jumps over the lazy dog
        encode 0
        output_Scaffold 0
        gather_All_Nodes 0

    Output:

        0 1 0 0 1
        1 3 0 32 1 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 t
        2 3 0 33 0 1 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 h
        3 4 0 34 0 0 1 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 0 0 0 0 1 28 1 e
        4 9 0 40 0 0 0 1 28 0 0 0 0 0 1 28 0 0 0 0 0 1 28 0 0 0 1 28 0 0 0 0 0 1 28 0 0 0 0 1 28 0 0 0 1 28 0 0 0 0 1 28 1
        5 2 0 5 0 0 0 0 1 28 1 q
        6 3 0 22 0 0 0 0 0 1 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 u
        7 2 0 7 0 0 0 0 0 0 1 28 1 i
        8 2 0 8 0 0 0 0 0 0 0 1 28 1 c
        9 2 0 9 0 0 0 0 0 0 0 0 1 28 1 k
        10 2 0 11 0 0 0 0 0 0 0 0 0 0 1 28 1 b
        11 3 0 30 0 0 0 0 0 0 0 0 0 0 0 1 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 r
        12 5 0 42 0 0 0 0 0 0 0 0 0 0 0 0 1 28 0 0 0 0 1 28 0 0 0 0 0 0 0 0 1 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 o
        13 2 0 14 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 w
        14 2 0 15 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 n
        15 2 0 17 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 f
        16 2 0 19 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 x
        17 2 0 21 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 j
        18 2 0 23 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 m
        19 2 0 24 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 p
        20 2 0 25 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 s
        21 2 0 28 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 v
        22 2 0 36 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 l
        23 2 0 37 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 a
        24 2 0 38 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 z
        25 2 0 39 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 y
        26 2 0 41 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 d
        27 2 0 43 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 28 1 g
        28 2 43 1 2 3 4 5 6 7 8 9 4 10 11 12 13 14 4 15 12 16 4 17 6 18 19 20 4 12 21 3 11 4 1 2 3 4 22 23 24 25 4 26 12 27 0 43 the quick brown fox jumps over the lazy dog

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Whitespace is weird when doing text input through set_Input! It reads from the text file using "file >> tmp_Variable", then this variable is concatenated to the others, "full_Variable = full_Variable + " " + tmp_Variable. If the next token is the '/end/' tag it does not append whitespace to the token, so 'set_Input 0 10 /end/' would only input '10' not '10 '. This full string is then read in as input and the network evaluates each character including spaces, even though whitespace was ignored in tokenization. This means that if you are trying to search for multiple tokens in a single string you need to be aware that spaces between the words may cause false positive associations due to the space or other whitespace. The greenscreen masking technique detailed in set_Input can be used to handle this behavior through input formatting only. So be aware these two behaviours stack to form a strange and peculiar action.
    */
    void encode(std::ifstream* p_File)
    {
        std::cout << "\n --> encode CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.encode(tmp_Construct);

        //std::cout << " [|x|]";
    }


    
    //		
        //Passes the values to an Construct to encode.
    /** Queries the network with the current input set for the given construct, evaluates the input set and stores the results in the output traces, gathered using 'gather_Output'.

        query CONSTRUCT_ID
    \param int CONSTRUCT_ID The construct that encapsulates the state space to search.
    \retval None This function doesn't return any values, the data resulting from this function is handled through traces.

    This sets up the scaffold as encode does, but it doesn't create nodes if they aren't found, they remain NULL in the scaffold, this we call a NULLCAN

    Used for querying the network, you input, fill the NULLCAN, charge the network, then gather the outputs. Does not create new nodes, only finds if they already exist or not, does not "train" only "evaluates".

    Suggested for use before encoding (if using learning mode and not locked_to_initial_training_mode) otherwise it will also find the current trace as the perfect match.

    To query a construct you must first have input loaded into it. Once the input is loaded then calling this will query the input pattern for a given construct. The way to use this is to first get input to the construct, then you tell it to evaluate the input, then you can use gather, output, and other functions to access the results.:

         Get Input: set_Input | set_Input_uint | load_Input
         Do something with the Input: query
         Get your Output: gather_X | output_X

    Example Usage:

    This example is a Many_To_One, which means all state nodes (Tier[0]) are connected to a single upper tier (Tier[1]) treetop node.

    You may notice that all the names are lowercase, this is because 'R' and 'r' are different symbols, so making them all lowercase keeps them using the same 'letters'.

    First we create a construct of the type Many_To_One named ExaCon for Example Construct.
    
        register_Construct Many_To_One ExaCon

    Then we use set_Input + encode to load several character names into the network and encode them as a base set, see encode for further details.
    
        set_Input 0 randolph carter /end/ encode 0
        set_Input 0 harley warren /end/ encode 0
        set_Input 0 herbert west /end/ encode 0
        set_Input 0 dr. allen halsey /end/ encode 0
        set_Input 0 erich zann /end/ encode 0
        set_Input 0 edward norrys /end/ encode 0
        set_Input 0 king kuranes /end/ encode 0

    After we have the base character names we can begin searching them. We set the input to the string 'i' to search names that have 'i' in them.
    
        set_Input 0 i

    To call this function and the correct construct we use 'query 0', this fills the NULLCAN and handles the neural cascades.
    
        query 0

    We then use 'output_Scaffold' to view the NULLCAN showing the tiny CAN holding the current queries, 'gather_Output 0' to output the resulting traces to the output file for Construct [0] (default ".\Output\CONSTRUCT_NAME.Output.ssv").
    
        output_Scaffold 0
        gather_Output 0

    To show different searches we do several searches with a newline in the output between them.

    Search for 'a':

        output_Newline 0
        set_Input 0 a
        query 0
        output_Scaffold 0
        gather_Output 0

    Search for 'h':
    
        output_Newline 0
        set_Input 0 h
        query 0
        output_Scaffold 0
        gather_Output 0

    Search for 'e':

        output_Newline 0
        set_Input 0 e
        query 0
        output_Scaffold 0
        gather_Output 0

    Search for 'eai':

        output_Newline 0
        set_Input 0 eai
        query 0
        output_Scaffold 0
        gather_Output 0

    Here is the above code in completion if you want to copy/paste:

        register_Construct Many_To_One ExaCon
        set_Input 0 randolph carter /end/ encode 0
        set_Input 0 harley warren /end/ encode 0
        set_Input 0 herbert west /end/ encode 0
        set_Input 0 dr. allen halsey /end/ encode 0
        set_Input 0 erich zann /end/ encode 0
        set_Input 0 edward norrys /end/ encode 0
        set_Input 0 king kuranes /end/ encode 0
        set_Input 0 i
        query 0
        output_Scaffold 0
        gather_Output 0
        output_Newline 0
        set_Input 0 a
        query 0
        output_Scaffold 0
        gather_Output 0
        output_Newline 0
        set_Input 0 h
        query 0
        output_Scaffold 0
        gather_Output 0
        output_Newline 0
        set_Input 0 e
        query 0
        output_Scaffold 0
        gather_Output 0
        output_Newline 0
        set_Input 0 eai
        query 0
        output_Scaffold 0
        gather_Output 0

    Output:

    CAN Scaffold for the 'i' search

        __COMMAND__| output_Scaffold CONSTRUCT_ID |
         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 22
        <- Tier[1] ->
          [0] NULL

    CAN Scaffold for the 'a' search

        __COMMAND__| output_Scaffold CONSTRUCT_ID |
         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 2
        <- Tier[1] ->
          [0] NULL

    CAN Scaffold for the 'h' search

        __COMMAND__| output_Scaffold CONSTRUCT_ID |
         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 8
        <- Tier[1] ->
          [0] NULL

    CAN Scaffold for the 'e' search

        __COMMAND__| output_Scaffold CONSTRUCT_ID |
         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 12
        <- Tier[1] ->
          [0] NULL

    CAN Scaffold for the 'eai' search

         __COMMAND__| output_Scaffold CONSTRUCT_ID |
          --==   CAN_Scaffold   ==--
         <- Tier[0] ->
           [0]  <> 12
           [1]  <> 2
           [2]  <> 22
         <- Tier[1] ->
           [0] NULL

    Contents of ".\Output\ExaCon.Output.ssv"

    Results of 'i' search

        29 10 2 12 king kuranes
        24 10 2 10 erich zann

    Results of 'a' search

        13 20 2 15 randolph carter
        16 20 2 13 harley warren
        25 10 2 13 edward norrys
        21 20 2 16 dr. allen halsey
        24 10 2 10 erich zann
        29 10 2 12 king kuranes

    Results of 'h' search

        16 10 2 13 harley warren
        19 10 2 12 herbert west
        24 10 2 10 erich zann
        13 10 2 15 randolph carter
        21 10 2 16 dr. allen halsey

    Results of 'e' search

        24 10 2 10 erich zann
        25 10 2 13 edward norrys
        19 30 2 12 herbert west
        16 20 2 13 harley warren
        21 20 2 16 dr. allen halsey
        29 10 2 12 king kuranes
        13 10 2 15 randolph carter

    Results of 'eai' search

        24 30 2 10 erich zann
        25 20 2 13 edward norrys
        19 30 2 12 herbert west
        16 40 2 13 harley warren <- the charge is 40 because it has 2 'a' characters, and 2 'e' characters.
        21 40 2 16 dr. allen halsey
        29 30 2 12 king kuranes
        13 30 2 15 randolph carter

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:
    - Use characters or numbers that you know won't appear in the dataset to create a "greenscreen" over your input for the network. This is due to the NULLCAN, where a node that isn't found is set to NULL on the CAN scaffold, and subsequently is skipped during charging as there is nothing to charge. Alternatively just use the query_Given_Legs command.
    - Whitespace is weird when doing text input through set_Input! It reads from the text file using "file >> tmp_Variable", then this variable is concatenated to the others, "full_Variable = full_Variable + " " + tmp_Variable. If the next token is the '/end/' tag it does not append whitespace to the token, so 'set_Input 0 10 /end/' would only input '10' not '10 '. This full string is then read in as input and the network evaluates each character including spaces, even though whitespace was ignored in tokenization. This means that if you are trying to search for multiple tokens in a single string you need to be aware that spaces between the words may cause false positive associations due to the space or other whitespace. The greenscreen masking technique detailed in set_Input can be used to handle this behavior through input formatting only. So be aware these two behaviours stack to form a strange and peculiar action.
    */
    void query(std::ifstream* p_File)
    {
        std::cout << "\n --> query CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.query(tmp_Construct);
    }

    /** Queries the network with the current input set of the given Construct, but the input index determines what leg is charged. Meaning if an input at index [3] is charged then only upper tier nodes connected on axon hillock [3] will be charged.

        query_Spacial CONSTRUCT_ID
    \param int CONSTRUCT_ID The construct that encapsulates the state space to search.
    \retval None This function doesn't return any values, the data resulting from this function is handled through traces.

    This query is used when the position of the input relative to the rest matters. Such as in time series searches, or context dependent inputs. The input position determines which connections get charged, this allows for searching nodes based on both the state of the input and the position.

    This would be useful for things such as time-series prediction, such as stock data or an environmental variable. 

    Example Usage:

    - Example 1.

    If we were trying to predict temperatures based on the previous temperatures and their fluctuations we might use this function. For example, if we have the current set of temperatures as { 50, 55, 60 } then we would want to know what comes after 60. First though we'll encode a sample history to search through.

    Here's the sample history:

        50 55 60 65 70 70 65 60 55 50

    We'll do this by encoding 3 temperatures at a time starting at the beginning of the sample history.

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 5 3 50 55 60
        encode 0
        set_Input_uint 0 5 3 55 60 65
        encode 0
        set_Input_uint 0 5 3 60 65 70
        encode 0
        set_Input_uint 0 5 3 65 70 70
        encode 0
        set_Input_uint 0 5 3 70 70 65
        encode 0
        set_Input_uint 0 5 3 70 65 60
        encode 0
        set_Input_uint 0 5 3 65 60 55
        encode 0
        set_Input_uint 0 5 3 60 55 50
        encode 0

    Assuming we encoded 3 temperatures at a time we can create a prediction by setting the first two indexes and searching "forward".

    So we input the set { 55, 60 } for the first two numbers, then search using this query_Spacial.

        set_Input_uint 0 5 2 55 60
        query_Spacial 0
        gather_Output_uint 0

    This means that only patterns with 55 as position [0], and 60 as position [1] will be found. This means those found will have position [2] as the "next" temperature.

    Output:

        6 20 2 3 55 60 65
        12 10 2 3 65 60 55

    We may choose to evaluate them through simple averaging the predicted index (number [2]). Or, alternatively we can weight them based on charge, or both.

    In this case we can choose the trace with the charge of 20 for our prediction, which is { 55, 60, 65 }

    A pure average would return 60, so trace selection is an important aspect that goes beyond this simple example. The choice on how to select the traces and filter them can make or break an algorithm.

    So we take our current input and add the new trace to get a prediction:

        55 60 ?? <- Current, the ?? is the index we are searching for, the prediction.
        55 60 65 <- Found
        =
        55 60 65 <- Resulting predicted pattern.

    - Example 2.

    For an example that hopefully illistrates the purpose of this function we can walk through searching for a string when you know only a couple characters and their position in the string. Pretend you are sleuthing around online hunting down pedo scum, suddenly discord dings, a message! One of your contacts has a lead, a screenshot from a twitch stream with the kingpin distributor you've been tracking for a while now. He's in this twitch chat, you know that avatar, but there's a problem. The chat animations have caused a cloud of emoji to be doubleing over the name right as the screenshot was taken. You have only the second and third characters, 'aw', but it is a lead.

    So we have the clue to work on [ '?' 'a' 'w' '?' ]. We know 'aw' are the second and third character, and the first char is a mystery, as are the rest if they even exist, which is liklely given how short that username would be.

    Luckily you have a dataset of usernames to look through, we'll train these into the network (obviously we are using a completely insufficient number but we aren't including 10k usernames here), then search the network using our clue.

    One thing to note, we have these usernames from sites we've scraped, and they have characters not allowed in the name. This means we can take our knowledge of how the network works and use this by inputting characters that cannot appear in usernames like a "greenscreen" in the string. The network during query doesn't create nodes, and if the username network never has these illegal characters encoded to a node then we will sure we are only searching for our intended characters, as no nodes will be in the scaffold for the invalid ones to charge.

    Code for our initial dataset, notice some names such as Flawless1337 do not have the 'aw' in the right spot:

        register_Construct Many_To_One ExaCon
        set_Input 0 SawBones9000 /end/ encode 0
        set_Input 0 MawOfD00M /end/ encode 0
        set_Input 0 PawPatriot1337 /end/ encode 0
        set_Input 0 DawgWalker2000 /end/ encode 0
        set_Input 0 TawTacticLol /end/ encode 0
        set_Input 0 RawRampage2001 /end/ encode 0
        set_Input 0 KawKungFuMaster /end/ encode 0
        set_Input 0 LawlessLegion1337 /end/ encode 0
        set_Input 0 Jawbreaker9001 /end/ encode 0
        set_Input 0 VawVandal2000 /end/ encode 0
        set_Input 0 AwkwardWarrior /end/ encode 0
        set_Input 0 Flawless1337 /end/ encode 0
        set_Input 0 StrawHat2000` /end/ encode 0
        set_Input 0 ClawClasherLol /end/ encode 0
        set_Input 0 GnawingGamer2001 /end/ encode 0
        set_Input 0 HacksawHavoc /end/ encode 0
        set_Input 0 Jawdropper2000 /end/ encode 0
        set_Input 0 MawMauler1337 /end/ encode 0
        set_Input 0 YawYawkerLol /end/ encode 0
        set_Input 0 ZawZapper2001 /end/ encode 0

    After the dataset we setup the query and submit it, using the 'illega' character of '_' to create "NULLCAN" nodes, which are positions in the current active node (CAN) scaffold, the datastructure which corrdinates encoding and charging, which don't have a node, therefore are "NULLCAN" nodes.

        set_Input 0 _aw
        query_Spacial 0

    Gather the output traces into the "./Output/ExaCon.ssv" file.

        gather_Output 0

    Put a newline in the output file so we can tell the difference between the two queries we are doing. Two queries to compare this function to the 'normal' query.

        output_Newline 0

    The input is still set to "_aw" so we don't have to reinput our string. We can instead call 'query 0' because the prep work is all done. We then gather the output as normal with 'gather_Output 0'.

        query 0
        gather_Output 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

    Compare the first set returned with this query_Spacial function here:

        11 20 2 12 SawBones9000
        16 20 2  9 MawOfD00M
        24 20 2 14 PawPatriot1337
        30 20 2 14 DawgWalker2000
        34 20 2 12 TawTacticLol
        38 20 2 14 RawRampage2001
        42 20 2 15 KawKungFuMaster
        43 20 2 17 LawlessLegion1337
        46 20 2 14 Jawbreaker9001
        49 20 2 13 VawVandal2000
        62 10 2 12 HacksawHavoc
        63 20 2 14 Jawdropper2000
        64 20 2 13 MawMauler1337
        66 20 2 12 YawYawkerLol
        68 20 2 13 ZawZapper2001

    To this set returned with the nondifferentiated query function:

        11 20 2 12 SawBones9000
        16 20 2  9 MawOfD00M
        24 30 2 14 PawPatriot1337
        30 30 2 14 DawgWalker2000
        34 30 2 12 TawTacticLol
        38 40 2 14 RawRampage2001 <- Notice the high charge of [40] due to the frequency of 'a' characters with the 'aw' sub-string.
        42 30 2 15 KawKungFuMaster
        43 20 2 17 LawlessLegion1337
        46 30 2 14 Jawbreaker9001
        49 40 2 13 VawVandal2000
        62 40 2 12 HacksawHavoc
        63 20 2 14 Jawdropper2000
        64 30 2 13 MawMauler1337
        66 40 2 12 YawYawkerLol
        68 30 2 13 ZawZapper2001
        52 20 2 12 Flawless1337
        58 30 2 14 ClawClasherLol
        60 30 2 16 GnawingGamer2001
        55 30 2 13 StrawHat2000`
        51 40 2 14 AwkwardWarrior

    Full code for the previous example if you wish to copy/paste:

        register_Construct Many_To_One ExaCon
        set_Input 0 SawBones9000 /end/ encode 0
        set_Input 0 MawOfD00M /end/ encode 0
        set_Input 0 PawPatriot1337 /end/ encode 0
        set_Input 0 DawgWalker2000 /end/ encode 0
        set_Input 0 TawTacticLol /end/ encode 0
        set_Input 0 RawRampage2001 /end/ encode 0
        set_Input 0 KawKungFuMaster /end/ encode 0
        set_Input 0 LawlessLegion1337 /end/ encode 0
        set_Input 0 Jawbreaker9001 /end/ encode 0
        set_Input 0 VawVandal2000 /end/ encode 0
        set_Input 0 AwkwardWarrior /end/ encode 0
        set_Input 0 Flawless1337 /end/ encode 0
        set_Input 0 StrawHat2000` /end/ encode 0
        set_Input 0 ClawClasherLol /end/ encode 0
        set_Input 0 GnawingGamer2001 /end/ encode 0
        set_Input 0 HacksawHavoc /end/ encode 0
        set_Input 0 Jawdropper2000 /end/ encode 0
        set_Input 0 MawMauler1337 /end/ encode 0
        set_Input 0 YawYawkerLol /end/ encode 0
        set_Input 0 ZawZapper2001 /end/ encode 0
        set_Input 0 _aw
        query_Spacial 0
        gather_Output 0
        output_Newline 0
        query 0
        gather_Output 0

    - Example 3.

    There may be times when you have a set number of inputs and you want to search for any patterns with that state in that spot. For example, you have your input set from your greenhouse sensors, oxygen, carbon dioxide, temperature, humidity, etc.

    Each of these is an row in your input list:

    -# Oxygen
    -# Carbon Dioxide
    -# Temperature
    -# Humidity
    -# Etc

    They are read in as a string of values 1 - 5, "O2 Co2 Temp Hum Etc", and example with values may be "2 4 2 1 3" to give an idea what an input list might look like to the neural network.

    So you encode each list, each snapshot, once every hour or so as your standard datakeeping, in fact it is automated. We'll use these made up snapshots for an example dataset:

        register_Construct Many_To_One ExaCon <- Don't forget to register the construct.
        set_Input 0 3 2 4 1 5 /end/ encode 0
        set_Input 0 2 3 3 2 4 /end/ encode 0
        set_Input 0 4 1 2 5 3 /end/ encode 0
        set_Input 0 5 4 3 2 1 /end/ encode 0
        set_Input 0 1 5 2 4 3 /end/ encode 0
        set_Input 0 3 2 1 4 5 /end/ encode 0
        set_Input 0 4 3 2 5 1 /end/ encode 0
        set_Input 0 1 4 3 2 5 /end/ encode 0
        set_Input 0 2 3 4 1 5 /end/ encode 0
        set_Input 0 5 1 2 3 4 /end/ encode 0
        set_Input 0 3 4 2 1 5 /end/ encode 0
        set_Input 0 2 1 3 5 3 /end/ encode 0
        set_Input 0 4 5 1 2 3 /end/ encode 0
        set_Input 0 1 2 3 4 5 /end/ encode 0
        set_Input 0 3 2 5 1 4 /end/ encode 0
        set_Input 0 2 3 4 5 1 /end/ encode 0
        set_Input 0 4 5 2 3 1 /end/ encode 0
        set_Input 0 5 1 3 2 4 /end/ encode 0
        set_Input 0 1 4 5 3 2 /end/ encode 0
        set_Input 0 3 2 4 5 1 /end/ encode 0
        set_Input 0 2 1 5 3 3 /end/ encode 0
        set_Input 0 4 3 1 2 5 /end/ encode 0
        set_Input 0 5 4 2 1 3 /end/ encode 0
        set_Input 0 1 5 4 3 2 /end/ encode 0
        set_Input 0 3 2 1 5 4 /end/ encode 0
    
    Now, your boss comes in one day and says they need some data for a research project. They want every snapshop that has a humity value of 3 (everything here is 1 - 5 in range for simplicity) and a O2 of 2.

    So you formulate your input string, using the greenscreen illegal character mask again, where the first and last are set to '2' and '3' respectively: "2 _ _ _ 3". There is a major issue with this that may not be apparent right away, the spaces are counted as valid symbols by the network. So we need to invalidate them as well to remove them from the query, "2_______3"

    This is because the input array is treated as an array of characters, with the caveat that extra whitespace is ignored and condensed down to a single space, ' ', which can be odd if not expected.

        set_Input 0 2_______3 /end/

    Then use this query_Spacial function & gather the output:

        query_Spacial 0
        gather_Output 0

    Which gives us the output:

         8 10 2 9 2 3 3 2 4
        15 10 2 9 2 3 4 1 5
        18 20 2 9 2 1 3 5 3 <- The charge here is [20] because both of the nodes were found in the right location.
        22 10 2 9 2 3 4 5 1
        27 20 2 9 2 1 5 3 3
         9 10 2 9 4 1 2 5 3
        11 10 2 9 1 5 2 4 3
        19 10 2 9 4 5 1 2 3
        29 10 2 9 5 4 2 1 3

    To further this example we will search for a temperature with a value of 1, and humidity of 4 to see what was going on when it was a wee bit chilly with high humidity.

    First we output a newline to the file to separate our searches, then we set the input. After setting the input we can then query_Spacial and gather_Output as normal.

        output_Newline 0
        set_Input 0 ____1___4 /end/
        query_Spacial 0
        gather_Output 0

    This gives the following output:

        12 10 2 9 3 2 1 4 5
        19 10 2 9 4 5 1 2 3
        28 10 2 9 4 3 1 2 5
        31 20 2 9 3 2 1 5 4 <- This appears to be the only really good match.
         8 10 2 9 2 3 3 2 4
        16 10 2 9 5 1 2 3 4
        21 10 2 9 3 2 5 1 4
        24 10 2 9 5 1 3 2 4

    Using this you can selectively search your traces.

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Use characters or numbers that you know won't appear in the dataset to create a "greenscreen" over your input for the network. This is due to the NULLCAN, where a node that isn't found is set to NULL on the CAN scaffold, and subsequently is skipped during charging as there is nothing to charge. Alternatively just use the query_Given_Legs command.
    - Whitespace is weird when doing text input through set_Input! It reads from the text file using "file >> tmp_Variable", then this variable is concatenated to the others, "full_Variable = full_Variable + " " + tmp_Variable. If the next token is the '/end/' tag it does not append whitespace to the token, so 'set_Input 0 10 /end/' would only input '10' not '10 '. This full string is then read in as input and the network evaluates each character including spaces, even though whitespace was ignored in tokenization. This means that if you are trying to search for multiple tokens in a single string you need to be aware that spaces between the words may cause false positive associations due to the space or other whitespace. The greenscreen masking technique detailed in set_Input can be used to handle this behavior through input formatting only. So be aware these two behaviours stack to form a strange and peculiar action.
    */
    void query_Spacial(std::ifstream* p_File)
    {
        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.query_Spacial(tmp_Construct);

        //std::cout << " [|x|]";
    }

    /** Queries the network with the current input set, however, every input node is charged using the given index.

        query_Given_Index CONSTRUCT_ID INDEX
    \param int CONSTRUCT_ID The construct that encapsulates the state space to search.
    \param int INDEX This is the index to use when charging, if you are searching for patterns that have any of the input nodes in index [2] then you would use this parameter to specify leg [2].
    \retval None This function doesn't return any values, the data resulting from this function is handled through traces.

    If INDEX is [3] then all nodes in the input set will be charging using axon hillock [3].

    This is used when you have multiple inputs to the same index that you want to check. For example, maybe you want to check for traces grouped around one end the temperature scale rather than a single value.
    
    Example Usage:

    - Example 1.

    Following up on the idea of checking a range of thermometer readings from the description we have this example. If your temperature gauge registers 1 -5 then in this example you might want to look up the super-symbols, the encoded traces, the upper tier nodes, that have 4 & 5 encoded onto the index corresponding to temperature on your greenhouse input array.

    Here is an example input set from our example greenhouse sensors, oxygen, carbon dioxide, temperature, humidity, etc.

    Each of these is an row in your input list:

    -# Oxygen
    -# Carbon Dioxide
    -# Temperature <- We'll be searching this index for the temperature ranges using this function query_Given_Index
    -# Humidity
    -# Etc

    They are read in as a string of values 1 - 5, "O2 Co2 Temp Humidity Etc", and example with values may be "2 4 2 1 3" to give an idea what an input list might look like to the neural network.

    So you encode each list, for this example we'll say your greenhouse controller automatically logs these input lists. We'll use some of this log data for an example dataset:

        register_Construct Many_To_One ExaCon <- Don't forget to register the construct.
        set_Input 0 3 2 4 1 5 /end/ encode 0
        set_Input 0 2 3 3 2 4 /end/ encode 0
        set_Input 0 4 1 2 5 3 /end/ encode 0
        set_Input 0 5 4 3 2 1 /end/ encode 0
        set_Input 0 1 5 2 4 3 /end/ encode 0
        set_Input 0 3 2 1 4 5 /end/ encode 0
        set_Input 0 4 3 2 5 1 /end/ encode 0
        set_Input 0 1 4 3 2 5 /end/ encode 0
        set_Input 0 2 3 4 1 5 /end/ encode 0
        set_Input 0 5 1 2 3 4 /end/ encode 0
        set_Input 0 3 4 2 1 5 /end/ encode 0
        set_Input 0 2 1 3 5 3 /end/ encode 0
        set_Input 0 4 5 1 2 3 /end/ encode 0
        set_Input 0 1 2 3 4 5 /end/ encode 0
        set_Input 0 3 2 5 1 4 /end/ encode 0
        set_Input 0 2 3 4 5 1 /end/ encode 0
        set_Input 0 4 5 2 3 1 /end/ encode 0
        set_Input 0 5 1 3 2 4 /end/ encode 0
        set_Input 0 1 4 5 3 2 /end/ encode 0
        set_Input 0 3 2 4 5 1 /end/ encode 0
        set_Input 0 2 1 5 3 3 /end/ encode 0
        set_Input 0 4 3 1 2 5 /end/ encode 0
        set_Input 0 5 4 2 1 3 /end/ encode 0
        set_Input 0 1 5 4 3 2 /end/ encode 0
        set_Input 0 3 2 1 5 4 /end/ encode 0

    Now we need to find the temperatures, the { 4, 5 }. We do this by setting the input appropriately. 

    To formulate the input strings we create an input string from out input indexes with no whitespace between them, this is because each one is the smallest unit of information so we don't risk losing any by placing characters next to each other. Being single digit values 1-5 we can do that with this string based input:

        45

    This 45 is represented as a string internally so each digit is a separate integer value even though a human reads it as forty-five.

    Then formulate it into the set_Input string for the construct 0 (ExaCon as defined using 'register_Construct Many_To_One ExaCon' above) we are working with:

        set_Input 0 45 /end/

    Finally we query the network with 'query_Given_Index' to search the network for all traces containing one of { 4, 5 } on leg 4, which corresponds to 4. Temperature, so index/leg/dendrite/axon_hillock #4. You might wonder why we are setting it to index 4 and not 2 like one would expect. This is because we have to count each whitespace as an index in the input string, "1 2 3 4 5" equals '1' + ' ' + '2' + ' ' + '3' + ' ' + '4' + ' ' + '5'. 
    
    Here is the query string:
    
        query_Given_Index 0 4

    Then use 'gather_Output' to get the output:

        gather_Output 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

         7 10 2 9 3 2 4 1 5
        15 10 2 9 2 3 4 1 5
        22 10 2 9 2 3 4 5 1
        26 10 2 9 3 2 4 5 1
        30 10 2 9 1 5 4 3 2
        21 10 2 9 3 2 5 1 4
        25 10 2 9 1 4 5 3 2
        27 10 2 9 2 1 5 3 3

    - Example 2.

    This example builds on the above one to search for temperatures matching 1, 2, & 3.

        set_Input 0 123 /end/

    Then the query and gather:

        query_Given_Index 0 4
        gather_Output 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":
    
        12 10 2 9 3 2 1 4 5
        19 10 2 9 4 5 1 2 3
        28 10 2 9 4 3 1 2 5
        31 10 2 9 3 2 1 5 4
         9 10 2 9 4 1 2 5 3
        11 10 2 9 1 5 2 4 3
        13 10 2 9 4 3 2 5 1
        16 10 2 9 5 1 2 3 4
        17 10 2 9 3 4 2 1 5
        23 10 2 9 4 5 2 3 1
        29 10 2 9 5 4 2 1 3
         8 10 2 9 2 3 3 2 4
        10 10 2 9 5 4 3 2 1
        14 10 2 9 1 4 3 2 5
        18 10 2 9 2 1 3 5 3
        20 10 2 9 1 2 3 4 5
        24 10 2 9 5 1 3 2 4

    - Example 3.

    In this example we basically redo the same environmental search again, but this time we'll use the uint interface.

    Start with encoding, but with uint and the syntax that requires (set_Input_uint CONSTRUCT_ID COUNT INPUT ):

        register_Construct Many_To_One ExaCon <- Don't forget to register the construct.
        set_Input_uint 0 5 3 2 4 1 5 encode 0
        set_Input_uint 0 5 2 3 3 2 4 encode 0
        set_Input_uint 0 5 4 1 2 5 3 encode 0
        set_Input_uint 0 5 5 4 3 2 1 encode 0
        set_Input_uint 0 5 1 5 2 4 3 encode 0
        set_Input_uint 0 5 3 2 1 4 5 encode 0
        set_Input_uint 0 5 4 3 2 5 1 encode 0
        set_Input_uint 0 5 1 4 3 2 5 encode 0
        set_Input_uint 0 5 2 3 4 1 5 encode 0
        set_Input_uint 0 5 5 1 2 3 4 encode 0
        set_Input_uint 0 5 3 4 2 1 5 encode 0
        set_Input_uint 0 5 2 1 3 5 3 encode 0
        set_Input_uint 0 5 4 5 1 2 3 encode 0
        set_Input_uint 0 5 1 2 3 4 5 encode 0
        set_Input_uint 0 5 3 2 5 1 4 encode 0
        set_Input_uint 0 5 2 3 4 5 1 encode 0
        set_Input_uint 0 5 4 5 2 3 1 encode 0
        set_Input_uint 0 5 5 1 3 2 4 encode 0
        set_Input_uint 0 5 1 4 5 3 2 encode 0
        set_Input_uint 0 5 3 2 4 5 1 encode 0
        set_Input_uint 0 5 2 1 5 3 3 encode 0
        set_Input_uint 0 5 4 3 1 2 5 encode 0
        set_Input_uint 0 5 5 4 2 1 3 encode 0
        set_Input_uint 0 5 1 5 4 3 2 encode 0
        set_Input_uint 0 5 3 2 1 5 4 encode 0

    For the input search string we use the uint format of stating the number of elements and our inputs, so 'set_Input 45 /end/' becomes 'set_Input_uint 2 4 5':

        set_Input 0 2 4 5

    The different input methods result in different input indexes for the values, meaning our query is different. The uint version gives us the intuitive index of [2] for our temperature because each number is a sequential input, rather than input - space - input:

        query_Given_Index 0 2

    We gather using the uint interface so it outputs the integers to the textfile properly:

        gather_Output_uint 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

         6 10 2 5 3 2 4 1 5
        14 10 2 5 2 3 4 1 5
        21 10 2 5 2 3 4 5 1
        25 10 2 5 3 2 4 5 1
        29 10 2 5 1 5 4 3 2
        20 10 2 5 3 2 5 1 4
        24 10 2 5 1 4 5 3 2
        26 10 2 5 2 1 5 3 3

    - Example Meta-Explanation:

    The difference in the node IDs is due to the string interface not encoding the ' ' as the second character. With the input set of 'n n n n n', and each 'n' being a value 1-5, this means the second index in the string will be ' ' and read in as NID[2]. You can compare the patterns between the string version and the uint version to see the data is the same, but the node IDs are off by one:

    String version 'set_Input':

         7 10 2 9 3 2 4 1 5
        15 10 2 9 2 3 4 1 5
        22 10 2 9 2 3 4 5 1
        26 10 2 9 3 2 4 5 1
        30 10 2 9 1 5 4 3 2
        21 10 2 9 3 2 5 1 4
        25 10 2 9 1 4 5 3 2
        27 10 2 9 2 1 5 3 3

    uint Version 'set_Input_uint':

         6 10 2 5 3 2 4 1 5
        14 10 2 5 2 3 4 1 5
        21 10 2 5 2 3 4 5 1
        25 10 2 5 3 2 4 5 1
        29 10 2 5 1 5 4 3 2
        20 10 2 5 3 2 5 1 4
        24 10 2 5 1 4 5 3 2
        26 10 2 5 2 1 5 3 3


    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - Use characters or numbers that you know won't appear in the dataset to create a "greenscreen" over your input for the network. This is due to the NULLCAN, where a node that isn't found is set to NULL on the CAN scaffold, and subsequently is skipped during charging as there is nothing to charge. Alternatively just use the query_Given_Legs command.
    - Whitespace is weird when doing text input through set_Input! It reads from the text file using "file >> tmp_Variable", then this variable is concatenated to the others, "full_Variable = full_Variable + " " + tmp_Variable. If the next token is the '/end/' tag it does not append whitespace to the token, so 'set_Input 0 10 /end/' would only input '10' not '10 '. This full string is then read in as input and the network evaluates each character including spaces, even though whitespace was ignored in tokenization. This means that if you are trying to search for multiple tokens in a single string you need to be aware that spaces between the words may cause false positive associations due to the space or other whitespace. The greenscreen masking technique detailed in set_Input can be used to handle this behavior through input formatting only. So be aware these two behaviours stack to form a strange and peculiar action.
    */
    void query_Given_Index(std::ifstream* p_File)
    {
        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        int tmp_Index = 0;

        *p_File >> tmp_Index;

        std::cout << " " << tmp_Index << " |";

        API.query_Given_Index(tmp_Construct, tmp_Index);

        //std::cout << " [|x|]";
    }

    /** Queries the network using the current input set of the given Construct, however, an array of legs is passed that signifies whether or not each individual leg is to be charged and every input queried to all axon_hillocks/legs set to fire. 

        query_Given_Legs CONSTRUCT_ID LEG_COUNT LGS 
    \param int CONSTRUCT_ID The construct that encapsulates the state space to search.
    \param int LEG_COUNT The number of legs you are passing.
    \param bool LEGS  The array of legs as boolean values used to determine whether or not to charge nodes connected on that axon_hillock/dendrite_index.
    \retval None This function doesn't return any values, the data resulting from this function is handled through traces.

    This function works by taking this set of 'legs' which is a boolean array. It then takes every input of the construct and puts them in a pool of unorganized nodes. It then steps through the boolean array checking whether that step needs to be charged or not. If it needs charged the program then steps through every node in the pool having it charge any axons on the current index, which would be the axon_hillock to the node.

    So if we are on the second leg in the array then we would step through each node, go check the axon_hillock population count, and if any are found we charge them into the charging buffers. 
    
    This means you will search for the set of inputs you give, not in order but individually, on the legs you specify.

    Example Usage:

    - Example 1.

    In this example we'll use the scenario of running a game server such as Minecraft and needing to track down some hackers. You have the chatlogs which are immense, and you have the user account database. One of your mods remembers that the hacker had an x or z in the first character or four of their name.

    Using this function we can search for { 'X', 'Z', 'x','z' } on the first 4 characters of each name. So to start we need to encode the database of names by formatting them with the set_Input command:

        register_Construct Many_To_One ExaCon /end/ encode 0
        set_Input 0 PotatoChipCrunch /end/ encode 0
        set_Input 0 NotARealIdentity /end/ encode 0
        set_Input 0 BabyDollXOXO /end/ encode 0
        set_Input 0 SpaceCowboy /end/ encode 0
        set_Input 0 NinjaGuy22 /end/ encode 0
        set_Input 0 ElectroExile /end/ encode 0
        set_Input 0 ArcaneAdventurer /end/ encode 0
        set_Input 0 Xx_KawaiiAngel_xX /end/ encode 0
        set_Input 0 FireDragon99 /end/ encode 0
        set_Input 0 ProUser2024 /end/ encode 0
        set_Input 0 MySpaceStar /end/ encode 0
        set_Input 0 RealNameJane /end/ encode 0
        set_Input 0 xXx_P0t4t0Ch1p_xXx /end/ encode 0
        set_Input 0 PixelPilgrim /end/ encode 0
        set_Input 0 d4t4b4s3Qu33n /end/ encode 0
        set_Input 0 NeonNomad /end/ encode 0
        set_Input 0 ByteBandit23 /end/ encode 0
        set_Input 0 DataMinerX /end/ encode 0
        set_Input 0 -R4v3n-Cl4w- /end/ encode 0
        set_Input 0 G1tGuD /end/ encode 0
        set_Input 0 d00d!3_H4xx0r /end/ encode 0
        set_Input 0 -MysticMoon- /end/ encode 0
        set_Input 0 Vyrail_Vyxian /end/ encode 0
        set_Input 0 xX_D@rk_P@ssw0rd_Xx /end/ encode 0
        set_Input 0 8BitCh4rm3r /end/ encode 0
        set_Input 0 $p4c3C@d3t /end/ encode 0
        set_Input 0 W1z4rd0f0s /end/ encode 0
        set_Input 0 C0mm@nd_L1n3_H3ro /end/ encode 0
        set_Input 0 ScriptKiddieSupreme /end/ encode 0

    Now that we have the database to search we can formulate the input query, punch it, and gather the output.

        set_Input 0 XZxz /end/
        query_Given_Legs 0 4 1 1 1 1
        gather_Output 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        44 20 2 17 Xx_KawaiiAngel_xX
        59 30 2 18 xXx_P0t4t0Ch1p_xXx <- Notice how this one had 3 matches to the character pool 'XZxz' in the searched string "xXx_"
        78 20 2 19 xX_D@rk_P@ssw0rd_Xx
        60 10 2 12 PixelPilgrim
        86 10 2 10 W1z4rd0f0s

    - Example 2.

    Building on the previous example we have another mod say he saw one of them with a '-' or '_' so we'll expand the search:

        set_Input 0 Xx-_ /end/
        query_Given_Legs 0 4 1 1 1 1
        output_Newline 0
        gather_Output 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        44 30 2 17 Xx_KawaiiAngel_xX
        59 40 2 18 xXx_P0t4t0Ch1p_xXx <- With the addition of the '_' to the pool of states we find that this "xXx_" now has a match on all characters evaluated. Making the charge 40.
        78 30 2 19 xX_D@rk_P@ssw0rd_Xx
        60 10 2 12 PixelPilgrim
        68 10 2 12 -R4v3n-Cl4w-
        73 10 2 12 -MysticMoon-

    - Example 3.

    If we examine the last example we find the charges are dependent on the number of matches. 
    
    The query treats the input as a pool of states, rather than an input string. This means that an input with repeated characters or states will evalaute them each time it encounters them.

    Using this you can "weigh" inputs manually. If we want to weight the '-' character we cna input it multiple times like this:

        set_Input 0 Xx-----_ /end/ <- We input the '-' character 5 times.
        output_Newline 0
        query_Given_Legs 0 4 1 1 1 1
        gather_Output 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        44 30 2 17 Xx_KawaiiAngel_xX
        59 40 2 18 xXx_P0t4t0Ch1p_xXx
        78 30 2 19 xX_D@rk_P@ssw0rd_Xx
        60 10 2 12 PixelPilgrim
        68 50 2 12 -R4v3n-Cl4w-
        73 50 2 12 -MysticMoon-

    Notice how the last two have charges of 50 vs the previous example where they had 10? With Many_To_One constructs the upper tier node charge is directly proportionate to how many inputs are linked to it (and modified by weights and such but you get the point).

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void query_Given_Legs(std::ifstream* p_File)
    {
        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        int* tmp_Legs = NULL;
        tmp_Legs = new int[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Legs[cou_Index];

            std::cout << " " << tmp_Legs[cou_Index] << " |";
        }

        API.query_Given_Legs(tmp_Construct, tmp_Count, tmp_Legs);

        if (tmp_Legs != NULL) { delete [] tmp_Legs; tmp_Legs = NULL; }

        //std::cout << " [|x|]";
    }


    //Disabled this doxygen for the moment, I suspect this function was accidentally brought to the surface while high.
    /* This allows for passing unordered sets of node IDs to be charged. 

        submit_Set CONSTRUCT_ID COUNT INPUT_UINT 
    \param int CONSTRUCT_ID The construct who's charging buffer and output will be used.
    \param COUNT The number of node IDs (NIDs) being passed.
    \param INPUT_UINT  The array of NIDs to charge.
    \retval None This function doesn't return any values.

    This function takes a set of NIDs and then directly submits them to the charging buffers, no input, query, scaffold, etc. If you have the ID of the node(s) and want to directly charge them you can use this function.

    Example Usage:

    To start with we encode a sample dataset, output the node network to get the node IDs to use, and then we can submit_Nodes and gather the output.

    Output:

        output

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void submit_Set(std::ifstream* p_File)
    {
        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        int tmp_Count = 0;

        *p_File >> tmp_Count;

        std::cout << " " << tmp_Count << " |";

        uint64_t* tmp_Input = NULL;
        tmp_Input = new uint64_t[tmp_Count];

        for (int cou_Index = 0; cou_Index < tmp_Count; cou_Index++)
        {
            *p_File >> tmp_Input[cou_Index];

            std::cout << " " << tmp_Input[cou_Index] << " |";
        }

        API.submit_Set(tmp_Construct, tmp_Count, tmp_Input);

        if (tmp_Input != NULL) { delete[] tmp_Input; tmp_Input = NULL; }

        //std::cout << " [|x|]";
    }


    /** Gets the treetop node for a given construct and puts it in the output file for that construct.

        get_Treetop CONSTRUCT_ID

    \param CONSTRUCT_ID The construct you want the treetop from.
    \retval uint64_t Treetop  The treetop is output to the return.ssv file.

    This returns the treetop node for a given construct and outputs it to the RETURN_FILE. If you instead want it to output to the construct output then you use gather_Output_Node instead of this function.

    This treetop is the top node in the current CAN Scaffold. So if you encode "yargi blargi" then this treetop node in a Many_To_One construct would be on tier [1] of the CAN Scaffold at index [0] that represents "yargi blargi".

    Example Usage:

    First we encode a string into a Many_To_One:

        register_Construct Many_To_One ExaCon
        set_Input 0 Pull Your Circuit Breaker /end/
        encode 0

    Then we can get output the scaffold so you can see it, and then get the treetop:

        output_Scaffold 0
        get_Treetop 0

    Output:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 3
          [4]  <> 4
          [5]  <> 5
          [6]  <> 6
          [7]  <> 2
          [8]  <> 7
          [9]  <> 4
          [10]  <> 8
          [11]  <> 9
          [12]  <> 7
          [13]  <> 10
          [14]  <> 2
          [15]  <> 9
          [16]  <> 11
          [17]  <> 4
          [18]  <> 12
          [19]  <> 7
          [20]  <> 13
          [21]  <> 14
          [22]  <> 15
          [23]  <> 13
          [24]  <> 7
        <- Tier[1] ->
          [0]  <> 16

    Contents of "./Output/returned.ssv":

        16

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - This function is for prototyping for those planning on converting to C++ data handling logic rather than using output.ssv.
    */
    uint64_t get_Treetop_NID(std::ifstream* p_File)
    {
        std::cout << "\n --> get_Treetop_NID CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        uint64_t tmp_Return_Node = API.get_Treetop_NID(tmp_Construct);

        write_to_output(RETURN_FILE, tmp_Return_Node);

        return tmp_Return_Node;

        //std::cout << " [|x|]";
    }

    void write_Treetop_NID_To_Other_Input(std::ifstream* p_File)
    {
        std::cout << "\n --> write_Treetop_NID_To_Other_Input CONSTRUCT_ID_FROM CONSTRUCT_ID_TO |";

        std::string tmp_Construct_From = "";
        std::string tmp_Construct_To = "";

        *p_File >> tmp_Construct_From;

        std::cout << " " << tmp_Construct_From << " |";

        *p_File >> tmp_Construct_To;

        std::cout << " " << tmp_Construct_To << " |";

        API.write_Treetop_NID_To_Other_Input(tmp_Construct_From, tmp_Construct_To);
    }

    void chrono_Shift(std::ifstream* p_File)
    {
        std::cout << "\n --> chrono_Shift CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.chrono_Shift(tmp_Construct);
    }

    void add_Chrono(std::ifstream* p_File)
    {
        std::cout << "\n --> add_Chrono CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        uint64_t tmp_Chrono = 0;

        *p_File >> tmp_Chrono;

        std::cout << " " << tmp_Chrono << " |";

        API.add_Chrono(tmp_Construct, tmp_Chrono);
    }


    /** Gets a single trace from a given node, puts it in the output file for the given construct.

        gather_Given_Trace CONSTRUCT_ID NID

    \param CONSTRUCT_ID This is the ID of construct who's output file will have the trace written to it.
    \param NID The ID of the node from which the pattern, or trace, will be extracted usin the backpropagation functions.
    \retval None This function doesn't return any values, output is done through the output file of the given construct.

    Searches the network for the given NID and if found it outputs the pattern represented by the node to the output file of the specified construct.

    This is used when you want to retrieve encoded information.

    Example Usage:

    First we encode some information.

        register_Construct Many_To_One ExaCon
        set_Input 0 THE GAME /end/
        encode 0
        output_Scaffold 0

    Output:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 5
          [5]  <> 6
          [6]  <> 7
          [7]  <> 3
        <- Tier[1] ->
          [0]  <> 8

    Then we can extract it by using the NID from above:

        gather_Given_Trace 0 8

    Contents of "./Output/ExaCon.Output.ssv":

        8 0 2 8 THE GAME

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void gather_Given_Trace(std::ifstream* p_File)
    {
        std::cout << "\n| gather_Given_Trace   CONSTRUCT_ID   NID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";
        
        int tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.gather_Given_Trace(tmp_Construct, tmp_NID);

        //std::cout << " [|x|]";
    }



    void write_Given_Pattern_As_Number(std::ifstream* p_File)
    {
        std::cout << "\n| write_Given_Pattern_As_Number |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " CONSTRUCT_ID " << tmp_Construct << " |";
        
        int tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " NID " << tmp_NID << " |";

        API.write_Given_Pattern_As_Number(tmp_Construct, tmp_NID);

        //std::cout << " [|x|]";
    }


    //---==  gather_All_Traces [CONSTRUCT]  ==---//
    //
    /** Gathers all the traces through a given construct.

        gather_All_Traces CONSTRUCT_ID
    \param CONSTRUCT_ID This is the construct who's output file will be used to store the results of the query.
    \retval None This function doesn't return any values, it stores the results in the output file of the given construct.

    This iterates over the network and calls gather_Given_Trace for every node.

    Example Usage:

    We encode some data into our newly minted construct, and then get the traces from it:

        register_Construct Many_To_One ExaCon
        set_Input 0 ABRACADABRA /end/ encode 0
        set_Input 0 FIREBALL /end/ encode 0
        gather_All_Traces 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        0 0 1 1
        1 0 7 1 A
        2 0 4 1 B
        3 0 4 1 R
        4 0 2 1 C
        5 0 2 1 D
        6 0 2 11 ABRACADABRA
        7 0 2 1 F
        8 0 2 1 I
        9 0 2 1 E
        10 0 3 1 L
        11 0 2 8 FIREBALL

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void gather_All_Traces(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_All_Traces   CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_All_Traces(tmp_Construct);

        //std::cout << " [|x|]";
    }


    void gather_All_Traces_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_All_Traces_uint   CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_All_Traces_uint(tmp_Construct);

        //std::cout << " [|x|]";
    }


    /** Used to wipe the input array of a given construct.

        reset_Input CONSTRUCT_ID
    \param CONSTRUCT_ID The construct who's input we are to wipe.
    \retval None This function doesn't return any values.

    For oopsies and when you need to prep a construct for new input. 

    Example Usage:

    Let's setup a construct, set the input, output the input (lel), wipe the input, and output it again to confirm:

        register_Construct Many_To_One ExaCon
        set_Input 0 This is amazing wheee /end/ encode 0
        output_Input 0

    Output:

         --==   CAN_Input   ==--
        [0] > T <
        [1] > h <
        [2] > i <
        [3] > s <
        [4] >   <
        [5] > i <
        [6] > s <
        [7] >   <
        [8] > a <
        [9] > m <
        [10] > a <
        [11] > z <
        [12] > i <
        [13] > n <
        [14] > g <
        [15] >   <
        [16] > w <
        [17] > h <
        [18] > e <
        [19] > e <
        [20] > e <

    Now wipe the output and check it again!:

        wipe_Input 0
        output_Input 0

    Output:

        --==   CAN_Input   ==--

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void reset_Input(std::ifstream* p_File)
    {
        std::cout << "\n --> reset_Input CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.reset_Input(tmp_Construct);

        //std::cout << " [|x|]";
    }

    void reset_Output(std::ifstream* p_File)
    {
        std::cout << "\n --> reset_Output CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.reset_Output(tmp_Construct);

        //std::cout << " [|x|]";
    }


    /** Sets the index for the given Construct state_Node_Tree

        set_State_Nodes_Index CONSTRUCT_ID INDEX
    \param CONSTRUCT_ID The construct who's state node tree you are setting.
    \param INDEX The index you are setting the state tree to.
    \retval None This function doesn't return any values.

    Sets the index for the given Construct state_Node_Tree in the c_Node_Network::State_Nodes  fractal state tree array. This keeps the state spaces separate and discrete. Most networks you don't want to share state nodes. Some you do, that's where this function comes in, allows you to point a construct to the state tree you want.

    It is important to remember that the state tree indices and the constructs don't line up. Construct[1] likely doesn't have State_Tree[1]!

    Example Usage:

    Create 2 constructs to work with, we'll be pointing ConExa's State Tree at ExaCon's State Tree:

        register_Construct Many_To_One ExaCon
        register_Construct Many_To_One ConExa

    Set the ConExa to ExaCon:

        set_State_Nodes_Index 1 1

    Encode a string into ExaCon & output the scaffold, then encode a similar one in ConExa & output the scaffold. Comparing these you can see the shared nodes underneath.

        set_Input 0 Control_String Variable /end/
        encode 0
        output_Scaffold 0

        set_Input 1 Control_String New_Data /end/
        encode 1
        output_Scaffold 1

    All constructs share the underlying node network, so those NIDs are unique to the entire network, not just a single construct. This means when you see the same node in both scaffolds you are seeing the exact same node.

    Output:

    Construct[0]:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 5
          [5]  <> 2
          [6]  <> 6
          [7]  <> 7
          [8]  <> 8
          [9]  <> 4
          [10]  <> 5
          [11]  <> 9
          [12]  <> 3
          [13]  <> 10
          [14]  <> 11
          [15]  <> 12
          [16]  <> 13
          [17]  <> 5
          [18]  <> 9
          [19]  <> 13
          [20]  <> 14
          [21]  <> 6
          [22]  <> 15
        <- Tier[1] ->
          [0]  <> 16

    Construct[1]:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 5
          [5]  <> 2
          [6]  <> 6
          [7]  <> 7
          [8]  <> 8
          [9]  <> 4
          [10]  <> 5
          [11]  <> 9
          [12]  <> 3
          [13]  <> 10
          [14]  <> 11
          [15]  <> 17
          [16]  <> 15
          [17]  <> 18
          [18]  <> 7
          [19]  <> 19
          [20]  <> 13
          [21]  <> 4
          [22]  <> 13
        <- Tier[1] ->
          [0]  <> 20

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - State tree indices and the constructs don't line up. Construct[1] likely doesn't have State_Tree[1]!
    */
    void set_State_Nodes_Index(std::ifstream* p_File)
    {
        std::cout << "\n --> set_State_Nodes_Index CONSTRUCT_ID INDEX |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";
        
        int tmp_Index = 0;

        *p_File >> tmp_Index;

        std::cout << " " << tmp_Index << " |";

        API.set_State_Nodes_Index(tmp_Construct, tmp_Index);

        //std::cout << " [|x|]";
    }


    //    ---==  set_input [CONSTRUCT_ID] [INPUT_STRING]  ==---
    /** This command sets the input of a given construct to the given input string.

        set_input CONSTRUCT_ID INPUT_STRING
    \param CONSTRUCT_ID The construct who's input you wish to set.
    \param INPUT_STRING The input string, either terminates from endl, or the token '/end/'
    \retval None This function doesn't return any values.

    The sub-string '/end/' can be used as a string terminator. This is useful when you want to string multiple commands together, otherwise anything after the input command would be considered input. 'set_Input 0 Example_Text encode 0' would end up setting construct [0]'s input to 'Example_Text encode 0' and nothing would be encoded.

    Use this function to set the input of the construct you'll be either encoding or evaluating, or both.
    
    Use characters or numbers that you know won't appear in the dataset to create a "greenscreen" over your input for the network. This is due to the NULLCAN, where a node that isn't found is set to NULL on the CAN scaffold, and subsequently is skipped during charging as there is nothing to charge. Alternatively just use the query_Given_Legs command.
    
    Whitespace is weird when doing text input through set_Input! It reads from the text file using "file >> tmp_Variable", then this variable is concatenated to the others, "full_Variable = full_Variable + " " + tmp_Variable. If the next token is the '/end/' tag it does not append whitespace to the token, so 'set_Input 0 10 /end/' would only input '10' not '10 '. This full string is then read in as input and the network evaluates each character including spaces, even though whitespace was ignored in tokenization. This means that if you are trying to search for multiple tokens in a single string you need to be aware that spaces between the words may cause false positive associations due to the space or other whitespace. The greenscreen masking technique detailed in set_Input can be used to handle this behavior through input formatting only. So be aware these two behaviours stack to form a strange and peculiar action.

    Example Usage:

    - Example 1.

    We'll register a construct, then set the input, then output the input so we can see it.

        register_Construct Many_To_One ExaCon
        set_Input 0 Careful lads, I think there's a sni- /end/
        output_Input 0

    Output:

         --==   CAN_Input   ==--
        [0] > C <
        [1] > a <
        [2] > r <
        [3] > e <
        [4] > f <
        [5] > u <
        [6] > l <
        [7] >   <
        [8] > l <
        [9] > a <
        [10] > d <
        [11] > s <
        [12] > , <
        [13] >   <
        [14] > I <
        [15] >   <
        [16] > t <
        [17] > h <
        [18] > i <
        [19] > n <
        [20] > k <
        [21] >   <
        [22] > t <
        [23] > h <
        [24] > e <
        [25] > r <
        [26] > e <
        [27] > ' <
        [28] > s <
        [29] >   <
        [30] > a <
        [31] >   <
        [32] > s <
        [33] > n <
        [34] > i <
        [35] > - <

    - Example 2.

    We're going to show what happens if you forget the '/end/' on a compound statement with set_Input in it.

        register_Construct Many_To_One ExaCon
        set_Input 0 Example_Text encode 0
        output_Input 0

    Gives the output:

         --==   CAN_Input   ==--
        [0] > E <
        [1] > x <
        [2] > a <
        [3] > m <
        [4] > p <
        [5] > l <
        [6] > e <
        [7] > _ <
        [8] > T <
        [9] > e <
        [10] > x <
        [11] > t <
        [12] >   <
        [13] > e <
        [14] > n <
        [15] > c <
        [16] > o <
        [17] > d <
        [18] > e <
        [19] >   <
        [20] > 0 <

    Whereas including the '/end/' closing tag:

        set_Input 0 Example_Text /end/ encode 0
        output_Input 0

    Gives the output:

         --==   CAN_Input   ==--
        [0] > E <
        [1] > x <
        [2] > a <
        [3] > m <
        [4] > p <
        [5] > l <
        [6] > e <
        [7] > _ <
        [8] > T <
        [9] > e <
        [10] > x <
        [11] > t <

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int set_Input(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Input CONSTRUCT_ID INPUT |";

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

            std::cout << " " << tmp_In;

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
        API.set_Input(tmp_Construct, tmp_In_Full);

        //std::cout << " [|x|]";

        return 1;
    }

    /** Sets the input of a given construct to the given array of uint64_t values.

        set_Input_uint CONSTRUCT_ID COUNT INPUT 

    \param CONSTRUCT_ID The construct who's input will be getting set.
    \param COUNT The number of elements to expect in the Input  array.
    \param INPUT  The uint64_t array that will be read into the input of the given construct.
    \retval None This function doesn't return any values.

    This function is used when you have arrays of unsigned integer values to work with. Used for handling nodes with multi-sensory constructs and others that use nodes as their I/O states.

    Example Usage:

    - Example 1.

    Imagine we have an array of unsigned integers representing the number of deaths a team has in a video game. Say 4 players:

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 4 3 10 12 87
        output_Input_uint 0

    Output:

         --==   CAN_Input   ==--
        [0] > 3 <
        [1] > 10 <
        [2] > 12 <
        [3] > 87 <

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void set_Input_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> set_Input_uint CONSTRUCT_ID COUNT INPUT  |";

        int tmp_Depth = 0;
        uint64_t* tmp_In_Full = NULL;

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        //Get the depth
        *p_File >> tmp_Depth;

        std::cout << " " << tmp_Depth << " |";

        tmp_In_Full = new uint64_t[tmp_Depth];

        for (int cou_Index = 0; cou_Index < tmp_Depth; cou_Index++)
        {
            tmp_In_Full[cou_Index] = 0;
            *p_File >> tmp_In_Full[cou_Index];

            std::cout << " " << tmp_In_Full[cou_Index] << " |";
        }

        API.set_Input_uint(tmp_Construct, tmp_Depth, tmp_In_Full);

        //std::cout << " [|x|]";
    }


    void set_2D_Input_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> set_2D_Input_uint |";

        int tmp_X_Depth = 0;
        int tmp_Y_Depth = 0;
        uint64_t** tmp_In_Full = NULL;

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " CONSTRUCT_ID " << tmp_Construct << " |";

        //Get the depth
        *p_File >> tmp_X_Depth;

        std::cout << " X_COUNT " << tmp_X_Depth << " |";

        //Get the depth
        *p_File >> tmp_Y_Depth;

        std::cout << " Y_COUNT " << tmp_Y_Depth << " |";

        tmp_In_Full = new uint64_t*[tmp_X_Depth];
        for (int cou_X = 0; cou_X < tmp_X_Depth; cou_X++)
        {
            tmp_In_Full[cou_X] = new uint64_t[tmp_Y_Depth];

            for (int cou_Y = 0; cou_Y < tmp_Y_Depth; cou_Y++)
            {
                tmp_In_Full[cou_X][cou_Y] = 0;
                *p_File >> tmp_In_Full[cou_X][cou_Y];

                //---std::cout << " " << tmp_In_Full[cou_X][cou_Y] << " |";

                //std::cout << "\n I - [ " << cou_X << " ][ " << cou_Y << " ]: >" << tmp_In_Full[cou_X][cou_Y] << "<";
            }
        }

        API.set_2D_Input_uint(tmp_Construct, tmp_X_Depth, tmp_Y_Depth, tmp_In_Full);

        //std::cout << " [|x|]";
    }


    void set_3D_Input_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> set_3D_Input_uint |";

        int tmp_X_Depth = 0;
        int tmp_Y_Depth = 0;
        int tmp_Z_Depth = 0;
        uint64_t*** tmp_In_Full = NULL;

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " Construct " << tmp_Construct << " |";

        //Get the depth
        *p_File >> tmp_X_Depth;

        std::cout << " X_Depth " << tmp_X_Depth << " |";

        //Get the depth
        *p_File >> tmp_Y_Depth;

        std::cout << " Y_Depth " << tmp_Y_Depth << " |";

        //Get the depth
        *p_File >> tmp_Z_Depth;

        std::cout << " Z_Depth " << tmp_Z_Depth << " |";

        tmp_In_Full = new uint64_t**[tmp_X_Depth];
        for (int cou_X = 0; cou_X < tmp_X_Depth; cou_X++)
        {
            tmp_In_Full[cou_X] = new uint64_t*[tmp_Y_Depth];

            for (int cou_Y = 0; cou_Y < tmp_Y_Depth; cou_Y++)
            {
                tmp_In_Full[cou_X][cou_Y] = new uint64_t [tmp_Z_Depth];

                for (int cou_Z = 0; cou_Z < tmp_Z_Depth; cou_Z++)
                {
                    tmp_In_Full[cou_X][cou_Y][cou_Z] = 0;
                    *p_File >> tmp_In_Full[cou_X][cou_Y][cou_Z];

                    //std::cout << " " << tmp_In_Full[cou_X][cou_Y][cou_Z] << " |";

                    //std::cout << "\n I - [ " << cou_X << " ][ " << cou_Y << " ][ " << cou_Z << " ]: >" << tmp_In_Full[cou_X][cou_Y][cou_Z] << "<";
                }
            }
        }

        API.set_3D_Input_uint(tmp_Construct, tmp_X_Depth, tmp_Y_Depth, tmp_Z_Depth, tmp_In_Full);

        //std::cout << " [|x|]";
    }




    /** Outputs the scaffold as node IDs.

        output_Scaffold CONSTRUCT_ID
    \param CONSTRUCT_ID The ID of the construct who's scaffold we'll be outputting.
    \retval None This function doesn't return any values.

    When a trace, or input pattern, is encoded into the network we use a current active node scaffold (CAN) to do so. This CAN Scaffold can be output for human analysis, this splays the symbol open to your gaze, all sub-symbols and bound states there to play with.

    Example Usage:

    We'll encode a string into a many to one and output the scaffold:

        register_Construct Many_To_One ExaCon
        set_Input 0 supersymbol
        encode 0
        output_Scaffold 0

    Output:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 5
          [5]  <> 1
          [6]  <> 6
          [7]  <> 7
          [8]  <> 8
          [9]  <> 9
          [10]  <> 10
        <- Tier[1] ->
          [0]  <> 11

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Scaffold(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Scaffold CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        std::cout << "\n";
        API.output_Scaffold(tmp_Construct);
        std::cout << "\n";

        //std::cout << " [|x|]";
    }


    /** Outputs the input of the given construct to the console.

        output_Input CONSTRUCT_ID

    \param CONSTRUCT_ID The construct who's input we shall output.
    \retval None This function doesn't return any values.

    Pretty simple, outputs the input array of the given construct to the console.

    Example Usage:

    Let us create a construct, then set the input, then output the input which shall be 'kaput'.

        register_Construct Many_To_One ExaCon
        set_Input 0 kaput /end/
        output_Input 0

    Output:

         --==   CAN_Input   ==-
        [0] > k <
        [1] > a <
        [2] > p <
        [3] > u <
        [4] > t <

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Input(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Input CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.output_Input(tmp_Construct);

        //std::cout << " [|x|]";
    }


    /** Outputs the input of the given construct to the console, but as uint.

        output_Input_uint CONSTRUCT_ID

    \param CONSTRUCT_ID The construct who's input we shall output.
    \retval None This function doesn't return any values.

    Outputs the input array of the given construct to the console with the values displayed as uint64_t.

    Example Usage:

    Let us create a construct, then set the input, then output the input:

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 3 8999 9000 9001
        output_Input_uint 0

    Output:

         --==   CAN_Input   ==--
        [0] > 8999 <
        [1] > 9000 <
        [2] > 9001 <

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Input_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Input_uint CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.output_Input_uint(tmp_Construct);
    }


    /** The output trace set is output.

        output_Output CONSTRUCT_ID

    \param CONSTRUCT_ID This is the construct which has the output trace set output to the console.
    \retval None This function doesn't return any values.

    Takes the current output traces from a given construct and outputs them to the console. So after a query this allows you to output the results.

    Example Usage:

    Here we'll register a construct, encode two input sets, query it, and then finally show the useage of this function.

    Register, encode:

        register_Construct Many_To_One ExaCon
        set_Input 0 01234 /end/
        encode 0
        set_Input 0 56789 /end/
        encode 0

    Set input, query, then show this functions TRUE POWER:

        set_Input 0 45 /end/
        query 0
        output_Output 0

    Output:

         --==   Output_Traces [2]   ==--
         Trace[0]:  Pat_Depth: 5
        --  Charge: 10 RC: 2 Treetop: 6 Pattern:  0 1 2 3 4
         Trace[1]:  Pat_Depth: 5
        --  Charge: 10 RC: 2 Treetop: 12 Pattern:  5 6 7 8 9
          --==   /Output_Traces   ==--

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Output(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Output |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.output_Output(tmp_Construct);
    }


    /** The output trace set is output as uint64_t.

        output_Output_uint CONSTRUCT_ID

    \param CONSTRUCT_ID This is the construct which has the output trace set output to the console with the values displayed as uint64_t.
    \retval None This function doesn't return any values.

    Takes the current output traces from a given construct and outputs them to the console. So after a query this allows you to output the results. The results are displayed as uint64_t and not string.

    Example Usage:

    Here we'll register a construct, encode two uint64_t input arrays, query it, and then finally show the useage of this function.

    Register, encode:

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 5 0 1 2 3 4
        encode 0
        set_Input_uint 0 5 5 6 7 8 9
        encode 0

    Set input, query, then show this functions TRUE POWER:

        set_Input_uint 0 3 4 5 6
        query 0
        output_Output_uint 0

    Output:

         --==   Output_Traces [2]   ==--
         Trace[0]:  Pat_Depth: 5
        --  Charge: 10 RC: 2 Treetop: 6 Pattern:  0 1 2 3 4
         Trace[1]:  Pat_Depth: 5
        --  Charge: 20 RC: 2 Treetop: 12 Pattern:  5 6 7 8 9 <- Notice this one has a charge of 20, otherwise it looks the same as the string query.

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Output_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Output_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.output_Output_uint(tmp_Construct);
    }


    /** Outputs the scaffold as characters typecase from the ID.

        output_Scaffold_Char CONSTRUCT_ID
    \param CONSTRUCT_ID The ID of the construct who's scaffold we'll be outputting.
    \retval None This function doesn't return any values.

    When a trace, or input pattern, is encoded into the network we use a current active node scaffold (CAN) to do so. This CAN Scaffold can be output for human analysis, each node represented as a 'random' character, making it so that at a glance you can tell variance in node structures, and changes if watching a life network.

    Each address is typecast to a char to give a pseudo-unique look to each node. For monke brain.

    Example Usage:

    We'll encode a string into a many to one and output the scaffold:

        register_Construct Many_To_One ExaCon
        set_Input 0 supersymbol
        encode 0
        output_Scaffold_Char 0

    Output:

         - [ 0 ]: output_Scaffold_Char

        (0000014828425EC0)
        ÉαÇá@ÉαÉ0╨p
        ►

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    void output_Scaffold_Char(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Scaffold_Char |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        std::cout << "\n";
        API.output_Scaffold_Char(tmp_Construct);
        std::cout << "\n";

        //std::cout << " [|x|]";
    }

    void output_Scaffold_Symbols(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Scaffold_Symbols |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        std::cout << "\n";
        API.output_Scaffold_Symbols(tmp_Construct);
        std::cout << "\n";

        //std::cout << " [|x|]";
    }

    void output_Scaffold_Symbols_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Scaffold_Symbols_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        std::cout << "\n";
        API.output_Scaffold_Symbols_uint(tmp_Construct);
        std::cout << "\n";

        //std::cout << " [|x|]";
    }





    //      ---====================---
    //     ---======================---
    //    ---========================---
    //   ---==========================---
    //  ---==   NT4 specific hooks   ==---
    //   ---==========================---
    //    ---========================---
    //     ---======================---
    //      ---====================---



    //    ---======================================---
    //   ---========================================---
    //  ---==   Used to register new constructs.   ==---
    //   ---========================================---
    //    ---======================================---


    /** Registers a Construct with the neural net engine of the given architecture and name desired.

        register_Construct TYPE NAME

    \param TYPE The node network architecture, Many_To_One, pyramidal, etc.
    \retval int Outputs the ID of the newly constructed construct to "./Output/returns.ssv".

    Registers a construct with the engine. 
    This calls "register_Construct" with the c_Node_Network object NNet in the c_Construct, which:
    - Creates an instance of a CAN scaffold of the type specified.
    - Creates a fractal state tree for the construct.
    - The name is registered in an array.
    - These are all encapsulated in a "construct".

    Currently these are the types of constructs available:
    - Many_To_One

    Many_To_One:

        This type is only 2 tiers, the state tier, and one treetop node per trace. You encode all the state nodes, then submit them as one to "get_Upper_Tier_Node".

    Example Usage:

    We'll register 3 constructs, then output the constructs:

        register_Construct Many_To_One ExaCon
        register_Construct Many_To_One ConExa
        register_Construct Many_To_One ExaMSC

        output_constructs

    Output:

        __COMMAND__| output_constructs |
        [0]: ExaCon
        [1]: ConExa
        [2]: ExaMSC

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int register_Construct(std::ifstream* p_File)
    {
        std::cout << "\n --> register_Construct |";

        //int add_Construct(std::string p_Construct_Name)

        std::string tmp_Construct_Type = "";

        *p_File >> tmp_Construct_Type;

        std::cout << " " << tmp_Construct_Type << " |";

        std::string tmp_Construct_Name = "";

        *p_File >> tmp_Construct_Name;

        std::cout << " " << tmp_Construct_Name << " |";

        return API.register_Construct(tmp_Construct_Type, tmp_Construct_Name);

        //std::cout << " [|x|]";
    }



    //    ---=====================---
    //   ---=======================---
    //  ---==   Input handling.   ==---
    //   ---=======================---
    //    ---=====================---

    /** Loads the input file associated with the given construct into the input array.

        load_Input CONSTRUCT_ID
        
    \param CONSTRUCT_ID This is the construct for whom the input will be loaded.
    \retval None This function doesn't return any values.

    Be warned, this function treats the entire file as one input, it ignored newlines. Iterating through it loads each whitespace separated token into a string, "tmp_Full = tmp_Full + ' ' + tmp_Current_Token"

    Example Usage:

    To start with we register our construct:

        register_Construct Many_To_One ExaCon

    We create the input file for it and put our sample data into the file:

    Contents of "./Input/ExaCon.Input.ssv":

        yo ho ho
        and
        a
        bottle
        of
        rum

    Now using load_Input we can load this into the construct:

        load_Input 0
    
    And we can see the results:

        | load_Input |
         - [ 0 ]: yo
         - [ 1 ]: ho
         - [ 2 ]: ho
         - [ 3 ]: and
         - [ 4 ]: a
         - [ 5 ]: bottle
         - [ 6 ]: of
         - [ 7 ]: rum

    You can see how it treats the tokens with different kinds of whitespace, newlines and spaces treated the same. (Looking at "yo ho ho")

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int load_Input(std::ifstream* p_File)
    {
        std::cout << "\n --> load_Input |";

        //Outputs the input for the Construct.
        //output_Input(int p_Construct)

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        return API.load_Input(tmp_Construct);

        //std::cout << " [|x|]";
    }

    /** Loads the input file associated with the given construct into the input array as uint.

        load_Input_uint CONSTRUCT_ID
        
    \param CONSTRUCT_ID This is the construct for whom the input will be loaded.
    \retval None This function doesn't return any values.

    The first number in the input file should be the count of elements. The function will create a temporary array to hold these and load them.

    Example Usage:

    To start with we register our construct:

        register_Construct Many_To_One ExaCon

    We create the input file for it and put our sample data into the file:

    Contents of "./Input/ExaCon.Input.ssv":

        4 10 20
        20 40

    Now using load_Input we can load this into the construct:

        load_Input_uint 0
    
    And we can see the results:

        | load_Input_uint |
         Count: 4
         - [ 0 ]: 10
         - [ 1 ]: 20
         - [ 2 ]: 20
         - [ 3 ]: 40

    You can see how it treats the tokens with different kinds of whitespace, newlines and spaces treated the same. (Looking at "yo ho ho")

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int load_Input_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> load_Input_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        return API.load_Input_uint(tmp_Construct);

        //std::cout << " [|x|]";
    }


    //    ---==========================================================================---
    //   ---============================================================================---
    //  ---==   Different ways of gathering nodes, individually, and the entire set.   ==---
    //   ---============================================================================---
    //    ---==========================================================================---

    /** This outputs the patterns as string when given a node ID (NID).

        gather_Given_Node CONSTRUCT_ID NID

    \param CONSTRUCT_ID The ID of the construct to use for gathering the node, the output will go to the output file associated with the construct given.
    \param NID The node ID (NID) of the node who's pattern we will be gathering, or decoding.
    \retval string This function write the string represented by the given NID to an output file associated with the given Construct.

    This is used when you want to retrieve a pattern from a NID. 

    Example Usage:

    The first step is to encode a pattern, into a newly minted construct for this example though it doesn't have to be new, it can be an old heirloom construct passed down through the generations:

        register_Construct Many_To_One ExaCon
        set_Input 0 The Dream Quest of Unkown Kadath /end/
        encode 0
        output_Scaffold

    Which shows us:

    Output:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 5
          [5]  <> 6
          [6]  <> 3
          [7]  <> 7
          [8]  <> 8
          [9]  <> 4
          [10]  <> 9
          [11]  <> 10
          [12]  <> 3
          [13]  <> 11
          [14]  <> 12
          [15]  <> 4
          [16]  <> 13
          [17]  <> 14
          [18]  <> 4
          [19]  <> 15
          [20]  <> 16
          [21]  <> 17
          [22]  <> 13
          [23]  <> 18
          [24]  <> 16
          [25]  <> 4
          [26]  <> 19
          [27]  <> 7
          [28]  <> 20
          [29]  <> 7
          [30]  <> 12
          [31]  <> 2
        <- Tier[1] ->
          [0]  <> 21

    From this we can see the upper tier node is '21'. So we tell it to retrieve that node, however, we create a new construct and get the node through that construct to show that as long as the construct is the same type we can retrieve patterns encoded into the nodes:

        register_Construct Many_To_One RandomCon
        gather_Given_Node 1 21

    For an output of:

    Contents of file "./Output/RandomCon.Output.ssv":

        21 2 32 1 2 3 4 5 6 3 7 8 4 9 10 3 11 12 4 13 14 4 15 16 17 13 18 16 4 19 7 20 7 12 2 0 32 The Dream Quest of Unkown Kadath

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Given_Node(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Given_Node   CONSTRUCT_ID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        int tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.gather_Given_Node(tmp_Construct, tmp_NID);


        //std::cout << " [|x|]";

        return 1;
    }


    /** This outputs the patterns as an array of uint64_t when given a node ID (NID).

        gather_Given_Node_uint CONSTRUCT_ID NID

    \param CONSTRUCT_ID The ID of the construct to use for gathering the node, the output will go to the output file associated with the construct given.
    \param NID The node ID (NID) of the node who's pattern we will be gathering, or decoding.
    \retval string This function write the string represented by the given NID to an output file associated with the given Construct.

    This is used when you want to retrieve a pattern from a NID as an array of uint64_t.

    Example Usage:

    In this example we will encode two lower tier constructs to get the NIDs, and feed them into an MSC and get a single treetop node that represents the super-symbol of all the input encoded into a single retrievable trace. We wil then use this to get the original input patterns back out.

    3 networks must be crafted with care, two raw tier constructs, and one higher multi-sensory construct (MSC):

        register_Construct Many_To_One ExaCon
        register_Construct Many_To_One ConExa
        register_Construct Many_To_One ExaMSC

    Now we'll encode the two raw constructs ExaCon and ConExa. ExaCon will be temperature, and ConExa will be oxygen:

        set_Input 0 Temperature: 2 /end/
        encode 0
        set_Input 1 Oxygen: 4 /end/
        encode 1

    Then we use gather_Treetop_Node function to get both treetops, and to output the raw node for each so we can see the full data to confirm this function works:

        gather_Treetop_NID 0
        output_Newline 0
        gather_Treetop_Node 0

        gather_Treetop_NID 1
        output_Newline 1
        gather_Treetop_Node 1

    Contents of "./Output/ExaCon.Output.ssv":

        12
        12 2 14 1 2 3 4 2 5 6 7 8 5 2 9 10 11 0 14 Temperature: 2

    Contents of "./Output/ConExa.Output.ssv":

        22
        22 2 9 13 14 15 16 17 18 19 20 21 0 9 Oxygen: 4

    Now that we have these { 12, 22 } we can move onto the multi-sensory construct:

        set_Input_uint 2 2 12 22
        encode 2

    Gather the output:

        gather_Treetop_NID 2
        output_Newline 2
        gather_Treetop_Node_uint 2

    Contents of "./Output/ExaMSC.Output.ssv":

        25
        25 2 2 23 24 0 2 12 22

    FINALLY we can use this function to get the information back out, starting by wiping the output files:

        clear_Output 0
        clear_Output 1
        clear_Output 2

    Now we get the pattern from our highest abstraction, the treetop from ExaMSC, NID number 25. Moving forward we will only use information retrieved from the network to reconstruct symbol #25, no previous trained knowledge we will use, only meta and structural:

        gather_Given_Node_uint 0 25

    Which gives us the file contents (in ExaCon! Which isn't the construct which encoded the pattern, to show the network is shared):

    Contents of "./Output/ExaCon.Output.ssv":

        25 2 2 23 24 0 2 12 22

    Looking at this trace we see { 2 12 22 } at the end, which means 2 elements, which are 12 & 22 repectively.

    We then output newline, gather the node 12, another newline, then the node 22. We use the default gather_Given_Node here to get the raw data, but we've shown the uint by using it to get this far:

        output_Newline 0
        gather_Given_Node 0 12
        output_Newline 0
        gather_Given_Node 0 22

    Results found in "./Output/ExaCon.Output.ssv":

        25 2 2 23 24 0 2 12 22
        12 2 14 1 2 3 4 2 5 6 7 8 5 2 9 10 11 0 14 Temperature: 2
        22 2 9 13 14 15 16 17 18 19 20 21 0 9 Oxygen: 4

    As you can see we have encoded and retrieved all patterns using this function for the MSC traces, and the gather_Given_Node for the raw.

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Given_Node_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Given_Node_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        int tmp_NID = 0;

        *p_File >> tmp_NID;

        std::cout << " " << tmp_NID << " |";

        API.gather_Given_Node_uint(tmp_Construct, tmp_NID);

        //std::cout << " [|x|]";

        return 1;
    }

    /** Gathers all of the nodes to the given constructs output.

        gather_All_Nodes CONSTRUCT_ID

    \param CONSTRUCT_ID The construct who's output will be used.
    \retval None This function doesn't return any values.

    Loops through each node and gathers them into the output file associated with the given construct.

    Example Usage:

    Register a construct and encode the string 'qwerty':

        register_Construct Many_To_One ExaCon
        set_Input 0 qwerty /end/
        encode 0

    Now use this function to gather all the nodes to the output file associated with the given construct:

        gather_All_Nodes 0


    Output:

    Contents of "./ExaCon.Output.ssv":

        0 1 0 0 1
        1 2 0 1 1 7 1 q
        2 2 0 2 0 1 7 1 w
        3 2 0 3 0 0 1 7 1 e
        4 2 0 4 0 0 0 1 7 1 r
        5 2 0 5 0 0 0 0 1 7 1 t
        6 2 0 6 0 0 0 0 0 1 7 1 y
        7 2 6 1 2 3 4 5 6 0 6 qwerty

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_All_Nodes(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_All_Nodes |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_All_Nodes(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }


    /** Gathers all of the nodes to the given constructs output as an array of uint64_t.

        gather_All_Nodes_uint CONSTRUCT_ID

    \param CONSTRUCT_ID The construct who's output will be used.
    \retval None This function doesn't return any values.

    Loops through each node and gathers them into the output file associated with the given construct, the output states/patterns are uint64_t.

    Example Usage:

    Register a construct and encode the array { 499, 500, 501 }:

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 3 499 500 501
        encode 0

    Now use this function to gather all the nodes to the output file associated with the given construct:

        gather_All_Nodes_uint 0


    Output:

    Contents of "./ExaCon.Output.ssv":

        0 1 0 0 1 0
        1 2 0 1 1 4 1 499
        2 2 0 2 0 1 4 1 500
        3 2 0 3 0 0 1 4 1 501
        4 2 3 1 2 3 0 3 499 500 501

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_All_Nodes_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_All_Nodes_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_All_Nodes_uint(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }


    //    ---==================================================================---
    //   ---====================================================================---
    //  ---==   The output of a given Construct is read into the output file.   ==---
    //   ---====================================================================---
    //    ---==================================================================---

    
    /** Takes every trace in the given Construct output trace array and writes them to the output file associated with that construct., note the output state patterns are treated as char.

        gather_Output CONSTRUCT_ID

    \param CONSTRUCT_ID The construct who's output is being gathered, or being written to the output file.
    \retval None This function doesn't return any values.

    Writes the output from a construct to the output file associated with that construct so that you can get the results of queries and other functions which result in a construct having traces in the output array.

    Example Usage:

    First we register our construct & encode a series of +- strings:

        register_Construct Many_To_One ExaCon
        set_Input 0 --- /end/ encode 0
        set_Input 0 --+ /end/ encode 0
        set_Input 0 -+- /end/ encode 0
        set_Input 0 -++ /end/ encode 0
        set_Input 0 +-- /end/ encode 0
        set_Input 0 +-+ /end/ encode 0
        set_Input 0 ++- /end/ encode 0
        set_Input 0 +++ /end/ encode 0

    Now we set the input and query it for the '+' char:

        set_Input 0 + /end/
        query 0

    Then to see the result we use gather_Output:

        gather_Output 0

    Output:

         7 10 2 3 +--
         8 20 2 3 +-+
         9 20 2 3 ++-
        10 30 2 3 +++
         5 10 2 3 -+-
         6 20 2 3 -++
         4 10 2 3 --+

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Output(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Output |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_Output(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }

    /** Takes every trace in the given Construct output trace array and writes them to the output file associated with that construct., note the output state patterns are treated as uint64_t.

        gather_Output_uint CONSTRUCT_ID

    \param CONSTRUCT_ID The construct who's output is being gathered, or being written to the output file.
    \retval None This function doesn't return any values.

    Writes the output from a construct to the output file associated with that construct so that you can get the results of queries and other functions which result in a construct having traces in the output array.

    Example Usage:

    First we register our construct & encode a series of 3 item arrays:

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 3 11 11 11 encode 0
        set_Input_uint 0 3 11 11 99 encode 0
        set_Input_uint 0 3 11 99 11 encode 0
        set_Input_uint 0 3 11 99 99 encode 0
        set_Input_uint 0 3 99 11 11 encode 0
        set_Input_uint 0 3 99 11 99 encode 0
        set_Input_uint 0 3 99 99 11 encode 0
        set_Input_uint 0 3 99 99 99 encode 0

    Now we set the input and query it for the '+' char:

        set_Input_uint 0 1 99
        query 0

    Then to see the result we use gather_Output:

        gather_Output_uint 0

    Output:

         7 10 2 3 99 11 11
         8 20 2 3 99 11 99
         9 20 2 3 99 99 11
        10 30 2 3 99 99 99
         5 10 2 3 11 99 11
         6 20 2 3 11 99 99
         4 10 2 3 11 11 99

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Output_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Output_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_Output_uint(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }


    //    ---=============================---
    //   ---===============================---
    //  ---==   Treetop node gathering.   ==---
    //   ---===============================---
    //    ---=============================---

    /** Gets the treetop node for a given construct and puts it in the output file for that construct as a raw node.

        gather_Treetop_Node CONSTRUCT_ID

    \param CONSTRUCT_ID The construct you want the treetop from.
    \retval c_Node Treetop  The treetop is output to the output file associated with the given construct.

    This returns the treetop node for a given construct and outputs it to the output file for the given construct.

    This treetop is the top node in the current CAN Scaffold. So if you encode "yargi blargi" then this treetop node in a Many_To_One construct would be on tier [1] of the CAN Scaffold at index [0] that represents "yargi blargi".

    Example Usage:

    First we encode a string into a Many_To_One:

        register_Construct Many_To_One ExaCon
        set_Input 0 Pull Your Circuit Breaker /end/
        encode 0

    Then we can get the treetop:

        gather_Treetop_Node 0

    Output:
    
    Contents of "./Output/ExaCon.Output.ssv":

        16 2 25 1 2 3 3 4 5 6 2 7 4 8 9 7 10 2 9 11 4 12 7 13 14 15 13 7 0 25 Pull Your Circuit Breaker

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Treetop_Node(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_treetop_node |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        //Gathers the treetop node.
        API.gather_Treetop_Node(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }

    /** Gets the treetop node for a given construct and puts it in the output file for that construct as a raw node, the output pattern will be uint64_t.

        gather_Treetop_Node_uint CONSTRUCT_ID

    \param CONSTRUCT_ID The construct you want the treetop from.
    \retval c_Node Treetop  The treetop is output to the output file associated with the given construct.

    This returns the treetop node for a given construct and outputs it to the output file for the given construct.

    This treetop is the top node in the current CAN Scaffold. So if you encode { 111, 222, 333 } then this treetop node in a Many_To_One construct would be on tier [1] of the CAN Scaffold at index [0] that represents { 111, 222, 333 }.

    Example Usage:

    First we encode a string into a Many_To_One:

        register_Construct Many_To_One ExaCon
        set_Input_uint 0 3 111 222 333
        encode 0

    Then we can get the treetop:

        gather_Treetop_Node_uint 0

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        4 2 3 1 2 3 0 3 111 222 333 

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Treetop_Node_uint(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_treetop_node_uint |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_Treetop_Node_uint(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }

    /** This takes the current treetop node from the CAN Scaffold (if one exists) and outputs the NID to the output file.

        gather_Treetop_NID CONSTRUCT_ID

    \param CONSTRUCT_ID The construct to retrieve the treetop node ID (NID) from.
    \retval uint64_t This function appends the NID of the treetop to the output file for the given construct.

    This function is useful when you are encoding multi-sensory constructs. When you encode a raw tier construct (one that deals with raw "sensory" data from outside the neural graph through state tier binding) the treetop is an abstraction of that sensory input. This function allows you to get a handle on that abstraction, a reference to that super-symbol.

    With this super-symbol or higher level abstraction you can feed it into higher tier constructs to build abstractions of abstractions with multiple lower constructs feeding their treetops into a higher tier construct.

    Example Usage:

    In this example we will encode two lower tier constructs to get the NIDs, and just for fun we'll feed them into an MSC to get a single treetop node that represents the super-symbol of all the input encoded into a single retrievable trace.

    First we create 3 networks, our old fren ExaCon, the somewhat familiar ConExa, and the rarely seen ExaMSC.

        register_Construct Many_To_One ExaCon
        register_Construct Many_To_One ConExa
        register_Construct Many_To_One ExaMSC

    Now we'll encode the two raw constructs ExaCon and ConExa. ExaCon will be temperature, and ConExa will be oxygen:

        set_Input 0 Temperature: 5 /end/
        encode 0
        set_Input 1 Oxygen: 3 /end/
        encode 1

    Output:

    The first construct ExaCon, temperature:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 2
          [5]  <> 5
          [6]  <> 6
          [7]  <> 7
          [8]  <> 8
          [9]  <> 5
          [10]  <> 2
          [11]  <> 9
          [12]  <> 10
          [13]  <> 11
        <- Tier[1] ->
          [0]  <> 12


    The second construct ConExa, oxygen:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 13
          [1]  <> 14
          [2]  <> 15
          [3]  <> 16
          [4]  <> 17
          [5]  <> 18
          [6]  <> 19
          [7]  <> 20
          [8]  <> 21
        <- Tier[1] ->
          [0]  <> 22

    Then we use this function to get both treetops, and to output the raw node for each so we can see the full data to confirm this function works:

        gather_Treetop_NID 0
        output_Newline 0
        gather_Treetop_Node 0

        gather_Treetop_NID 1
        output_Newline 1
        gather_Treetop_Node 1

    Output:

    Contents of "./Output/ExaCon.Output.ssv":

        12
        12 2 14 1 2 3 4 2 5 6 7 8 5 2 9 10 11 0 14 Temperature: 5

    Contents of "./Output/ConExa.Output.ssv":

        22
        22 2 9 13 14 15 16 17 18 19 20 21 0 9 Oxygen: 3

    Now that we have these { 12, 22 } we can move onto the multi-sensory construct:

        set_Input_uint 2 2 12 22
        encode 2
        output_Scaffold 2

    Gather the output:

        gather_Treetop_NID 2
		output_Newline 2
		gather_Treetop_Node_uint 2

    Contents of "./Output/ExaMSC.Output.ssv":

        25
        25 2 2 23 24 0 2 12 22

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int gather_Treetop_NID(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Treetop_NID |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        //Passes the values to an Construct to encode.
        API.gather_Treetop_NID(tmp_Construct);

        //std::cout << " [|x|]";

        return 1;
    }


    void gather_Treetops(std::ifstream* p_File)
    {
        std::cout << "\n --> gather_Treetops |";

        std::string tmp_Construct = "";

        *p_File >> tmp_Construct;

        std::cout << " " << tmp_Construct << " |";

        API.gather_Treetops(tmp_Construct);
    }


    //    ---======================================================================---
    //   ---========================================================================---
    //  ---==   Output the Construct input, output, scaffolds, node network, etc.   ==---
    //   ---========================================================================---
    //    ---======================================================================---

    /** This iterates through every node outputting them to the console.

        output_Node_Network

    \retval None This function doesn't return any values.

    This function iterates over every node in the network and outputs the to the console for you to view. This is not the patterns represented by the nodes, but the node structures themselves.

    Example Usage:

    First we register the construct, then we encode a series of binary values just because:

        register_Construct Many_To_One ExaCon
        set_Input 0 000 /end/ encode 0
        set_Input 0 001 /end/ encode 0
        set_Input 0 010 /end/ encode 0
        set_Input 0 011 /end/ encode 0
        set_Input 0 100 /end/ encode 0
        set_Input 0 101 /end/ encode 0
        set_Input 0 110 /end/ encode 0
        set_Input 0 111 /end/ encode 0

    Now output the network:

        output_Node_Network

    Output:

        | output_Node_Network |
         Node_ID (NID): 0  RC: 1 Type: 0 State { 0 } --- Dendrites:  --- Axon_Hillock_Count: 0
         Node_ID (NID): 1  RC: 13 Type: 0 State { 48 } --- Dendrites:  --- Axon_Hillock_Count: 3 Axons[0] <4>:  [0] 2 [1] 4 [2] 5 [3] 6 Axons[1] <4>:  [0] 2 [1] 4 [2] 7 [3] 8 Axons[2] <4>:  [0] 2 [1] 5 [2] 7 [3] 9
         Node_ID (NID): 2  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 1 [1] 1 [2] 1 --- Axon_Hillock_Count: 0
         Node_ID (NID): 3  RC: 13 Type: 0 State { 49 } --- Dendrites:  --- Axon_Hillock_Count: 3 Axons[0] <4>:  [0] 7 [1] 8 [2] 9 [3] 10 Axons[1] <4>:  [0] 5 [1] 6 [2] 9 [3] 10 Axons[2] <4>:  [0] 4 [1] 6 [2] 8 [3] 10
         Node_ID (NID): 4  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 1 [1] 1 [2] 3 --- Axon_Hillock_Count: 0
         Node_ID (NID): 5  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 1 [1] 3 [2] 1 --- Axon_Hillock_Count: 0
         Node_ID (NID): 6  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 1 [1] 3 [2] 3 --- Axon_Hillock_Count: 0
         Node_ID (NID): 7  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 3 [1] 1 [2] 1 --- Axon_Hillock_Count: 0
         Node_ID (NID): 8  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 3 [1] 1 [2] 3 --- Axon_Hillock_Count: 0
         Node_ID (NID): 9  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 3 [1] 3 [2] 1 --- Axon_Hillock_Count: 0
         Node_ID (NID): 10  RC: 2 Type: 2 State { 0 } --- Dendrites:  [0] 3 [1] 3 [2] 3 --- Axon_Hillock_Count: 0

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int output_Node_Network()
    {
        std::cout << "\n --> output_Node_Network |";

        //Passes the values to an Construct to encode.
        API.output_Node_Network();

        //std::cout << " [|x|]";

        return 1;
    }

    //    ---==  output_node  ==---
    /** Outputs a given node as defined by the passed NID to the console.

        function_name params
    \param
    \retval None This function doesn't return any values.

    The node is output to the console for the user to read, the symbol is backpropagated out and presented as both uint64_t and char().

    Example Usage:

    First we'll register a construct, then set & encode a string, then output the scaffold so we can see the nodes:
    
		register_Construct Many_To_One ExaCon
        set_Input 0 T-800 /end/
		encode 0
		output_Scaffold 0
		
    Output:

         --==   CAN_Scaffold   ==--
        <- Tier[0] ->
          [0]  <> 1
          [1]  <> 2
          [2]  <> 3
          [3]  <> 4
          [4]  <> 4
        <- Tier[1] ->
          [0]  <> 5

    Looking at this we can see that the treetop node (Tier[1] Index[0] in this Many_To_One) is NID #5, so we can output node number 5:

        output_Node 5

    Output:

         | output_Node | 5 |
        NID: 5 Type: 2 Symbol: <=- 5 * [ T, 84 ]  [ -, 45 ]  [ 8, 56 ]  [ 0, 48 ]  [ 0, 48 ]  -=>

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int output_Node(std::ifstream* p_File)
    {
        std::cout << "\n --> output_Node |";

        int tmp_Node = 0;

        *p_File >> tmp_Node;

        std::cout << " " << tmp_Node << " |";

        API.output_Backpropagated_Symbol_NID(tmp_Node);

        //std::cout << " [|x|]";

        return 1;
    }

    /** Outputs the constructs currently registered.

        output_constructs
    \retval None This function doesn't return any values.

    Outputs the list of currently registered constructs to the console.

    Example Usage:

    We'll register some constructs, then output them:

        register_Construct Many_To_One First_Conman
        register_Construct Many_To_One Second_Conman
        register_Construct Many_To_One Third_Conman
        output_constructs

    Output:

        __COMMAND__| output_constructs |
        [0]: First_Conman
        [1]: Second_Conman
        [2]: Third_Conman

    Error Handling:

    - No error handling is implemented in this function.

    Additional Notes:

    - None.
    */
    int output_Constructs()
    {
        std::cout << "\n --> output_Constructs |";

        API.output_Constructs();

        //std::cout << " [|x|]";

        return 1;
    }



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
        API.clear_Output(tmp_Construct);

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
        API.output_Newline(tmp_Construct);

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

        API.write_Text(tmp_Construct, tmp_In_Full);


        //std::cout << " [|x|]";

        return 1;
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


    /** brief

        function_name params
    \param 
    \retval None This function doesn't return any values.
	
	Describe behavior and side-effects. What it basically does and if it touches any globals.
	
	Example Usage:
	
	    function_name params
	
	Output:
	
	    output
	
	Error Handling:
	
	- No error handling is implemented in this function.
	
	Additional Notes:
	
	- None.
    */
    void save(std::ifstream* p_File)
    {
        std::cout << "\n --> save |";

        std::string tmp_FName;

        *p_File >> tmp_FName;

        std::cout << " " << tmp_FName << " |";

        tmp_FName = "./cores/" + tmp_FName;
        std::cout << " " << tmp_FName << " |";

        API.save(tmp_FName);

        //std::cout << " [|x|]";
    }


    void load(std::ifstream* p_File)
    {
        std::cout << "\n --> load |";

        std::string tmp_FName;

        *p_File >> tmp_FName;

        std::cout << " " << tmp_FName << " |";

        tmp_FName = "./cores/" + tmp_FName;
        std::cout << " " << tmp_FName << " |";

        API.load(tmp_FName);

        //std::cout << " [|x|]";
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

        std::string tmp_Message = "______________________________IDLING______________________________";
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

                for (int cou_Index = 0; cou_Index < (tmp_Message.length() + 9); cou_Index++)
                {
                    std::cout << char(8);
                }

                tmp_Corrupt_Message = tmp_Message;

                for (int cou_Index = 0; cou_Index < tmp_Distance; cou_Index++)
                {
                    tmp_Corrupt_Message[rand() % tmp_Message.length()] = char((std::rand() % 32) + 32);
                }

                std::cout << "STANDBY[" << tmp_Corrupt_Message << "]";
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

            }

            Tick++;
        }
    }

    void rcon()
    {

    }
};

/** @}*/