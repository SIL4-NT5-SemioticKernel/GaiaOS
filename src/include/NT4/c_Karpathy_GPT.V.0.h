//This was made by following along with the youtube video demonstrating from scratch building by Karpathy/nanoGPT, but using NT4 as the ML portion and tranlating the logic to NT4.

//This relies on the NT4 script: './Scripts/Karpathy_GPT_Core.txt'.
//autoexec must have 'Karpathy_GPT_Core.txt' to work on startup. .
class c_Karpathy_GPT
{
	//The length of tokens to read in at a time.
	int block_Size;

	//This is the total length of tokens to read in at once.
	//The context is read in once, the current block read in multiple times.
	int context_Size;

	//The current input string.
	uint64_t * Input_Tokens;

	//The current output string.
	std::string Output;

	c_Construct_API NT4_Core;

	//Where to save the core.
	std::string Save_FName;

public:

	c_Karpathy_GPT(const int p_Block_Size = 50, const int p_Context_Size = 500)
	{
		block_Size = p_Block_Size;
		context_Size = p_Context_Size;

		Save_FName = "./cores/Karpathy_GPT.ssv";

		Input_Tokens = new uint64_t[block_Size];

		for (int cou_Index = 0; cou_Index < block_Size; cou_Index++)
		{
			Input_Tokens[cou_Index] = 0;
		}

		std::cout << "\n\n";
		NT4_Core.register_Construct("Many_To_One", "Chrono");
		NT4_Core.register_Construct("Many_To_One", "Raw");
		NT4_Core.create_Construct_Connection("Raw", "Chrono");

		//This is a bit hacky, but atm we set the charing tier of the raw way above where it is reasonable.
		//This causes it to not actually charge, but it does fill the scaffold NULLCAN style with no reinforcement.
		NT4_Core.set_Charging_Tier("Raw", 100);
		NT4_Core.set_Action_Potential_Threshold("Chrono", 0.5);
	}

	void shift_Input()
	{
		for (int cou_Index = 0; cou_Index < block_Size; cou_Index++)
		{
			//---std::cout << "\n [ " << Input[cou_Index] << " " << Input[cou_Index + 1] << " ]";
			Input_Tokens[cou_Index] = Input_Tokens[cou_Index + 1];
		}
		Input_Tokens[block_Size - 1] = 0;
	}

	//We train at "block_Size + 1".
	int train_Textfile(const std::string p_FName)
	{
		char tmp_Char = ' ';

		int tmp_Train_Size = block_Size + 1;

		//We need to set the chrono input up to the correct depth.
		uint64_t* tmp_Chrono = new uint64_t[tmp_Train_Size];
		for (int cou_Index = 0; cou_Index < tmp_Train_Size; cou_Index++)
		{
			tmp_Chrono[cou_Index] = 0;
		}
		NT4_Core.set_Input_uint("Chrono", tmp_Train_Size, tmp_Chrono);
		//NT4_Core.output_Input("Chrono");

		delete[] tmp_Chrono;

		std::string tmp_FName = "./GPT/" + p_FName;

		std::ifstream IF(tmp_FName);

		if (!IF.is_open())
		{
			std::cerr << "\n   V(0.0)v ERROR: File not found: " << tmp_FName;
		}
		else
		{
			std::cout << "\n File Opened: " << tmp_FName;
		}

		std::string tmp_Input = "";
		int tmp_Check = 5000;
		int tmp_Current = 0;
		int tmp_Chrono_Current = 0;
		int tmp_Token_Count = 0;

		while (!(IF.eof()))
		{
			//if (!(tmp_Current % tmp_Check)) { std::cout << "\n..." << tmp_Current << "..."; }
			//tmp_Current++;

			// Process the character (e.g., print it)
			//---std::cout << tmp_Char;

			tmp_Input = "";
			IF >> tmp_Input;
			tmp_Token_Count++;
		}
		IF.close();

		IF.open(tmp_FName);

		if (!IF.is_open())
		{
			std::cerr << "\n   V(0.0)v ERROR: File not found: " << tmp_FName;
		}
		else
		{
			while (!(IF.eof()))
			{
				if (!(tmp_Current % tmp_Check)) { std::cout << "\n..." << tmp_Current << " / " << tmp_Token_Count << "..."; }
				tmp_Current++;

				// Process the character (e.g., print it)
				//---std::cout << tmp_Char;

				tmp_Input = "";
				IF >> tmp_Input;

				//---std::cout << "\n Input: <";
				//---std::cout << Input;
				//---std::cout << ">";

				if (tmp_Input != "")
				{
					NT4_Core.set_Input("Raw", tmp_Input);
					//---NT4_Core.output_Input("Raw");
					NT4_Core.encode("Raw");

					//---NT4_Core.output_Scaffold("Raw");
					NT4_Core.pull_Chrono_From_Lower_Connection("Chrono");
					//---NT4_Core.output_Input("Chrono");

					if (tmp_Chrono_Current > tmp_Train_Size)
					{
						NT4_Core.encode("Chrono");
					}
					//---NT4_Core.output_Scaffold("Chrono");

					tmp_Chrono_Current++;
				}
			}
			std::cout << "\n...Done...";
		}
		IF.close();
		return 1;
	}

	int evaluate_Text()
	{
		std::cout << "\n Evaluating Text...";

		//We need to set the chrono input up to the correct depth.
		uint64_t* tmp_Chrono = new uint64_t[block_Size];
		for (int cou_Index = 0; cou_Index < block_Size; cou_Index++)
		{
			tmp_Chrono[cou_Index] = 0;
		}
		NT4_Core.set_Input_uint("Chrono", block_Size, tmp_Chrono);
		NT4_Core.output_Input("Chrono");

		delete[] tmp_Chrono;

		//The input tokens are used to track the current input.
		//We use the input tokens as the network isn't encoding, but just querying.
		if (Input_Tokens != NULL) { delete[] Input_Tokens; Input_Tokens = NULL; }
		Input_Tokens = new uint64_t[block_Size];

		for (int cou_Index = 0; cou_Index < block_Size; cou_Index++)
		{
			Input_Tokens[cou_Index] = 0;
		}

		std::string tmp_Input = "";

		int tmp_Count = 0;

		while (tmp_Input != "/exit/")
		{
			NT4_Core.update_Config("Chrono");
			NT4_Core.update_Config("Raw");

			tmp_Count = 0;
			std::cout << "\n\n\n Filling Chrono...";
			std::cout << "\n\n\n[";
			std::string tmp_Seed_Text = "";
			std::string tmp_Output_Text = "";
			while (tmp_Count < block_Size)
			{
				//---std::cout << "\n [" << tmp_Count << " / " << (block_Size - 1) << "]";

				std::cin >> tmp_Input;

				if (tmp_Input == "/exit/") { return 1; }

				tmp_Seed_Text += tmp_Input;
				tmp_Seed_Text += " ";

				NT4_Core.set_Input("Raw", tmp_Input);
				NT4_Core.query("Raw");

				shift_Input();

				Input_Tokens[block_Size - 1] = NT4_Core.get_Treetop_NID("Raw");

				tmp_Count++;
			}
			std::cout << "] ";
			std::cout << "\n\n\n[" << tmp_Seed_Text << "]";
			for (int cou_Index = 0; cou_Index < 100; cou_Index++)
			{
				if (!(cou_Index % 25)) { std::cout << "\n"; }
				//std::cout << "\n [" << cou_Index << " / " << 50;

				NT4_Core.reset_Output("Chrono");
				NT4_Core.reset_Output("Raw");
				
				NT4_Core.set_Input_uint("Chrono", block_Size, Input_Tokens);
				//NT4_Core.output_Input("Chrono");

				NT4_Core.query_Spacial("Chrono");
				//NT4_Core.query_Spacial("Chrono");
				//NT4_Core.query("Chrono");

				NT4_Core.set_Input_uint("Chrono", 1, &(Input_Tokens[block_Size - 1]));
				NT4_Core.query_Given_Index("Chrono", (block_Size - 1));

				NT4_Core.gather_Treetops("Chrono");
				//NT4_Core.output_Output("Chrono");

				NT4_Core.pull_From_Upper_Index("Raw", "Chrono", (block_Size));
				//NT4_Core.output_Output("Raw");

				//std::cout << "\n\n Chrono Output: ";
				//NT4_Core.output_Output("Chrono");

				//std::cout << "\n\n Raw Output: ";
				//NT4_Core.output_Output("Raw");

				//std::cout << "\n\n\n Enter Token[" << cou_Index << "]: ";
				//std::cin >> tmp_Input;

				//Get the number of outputs.
				int tmp_Top = NT4_Core.get_Output_Depth("Raw");
				//std::cout << "\n " << tmp_Top << " number of tokens found.";

				//Loop through and pick 1000 at random keeping the highest one.
				double tmp_Highest_Charge = 0.0;

				for (int cou_Index = 0; cou_Index < 1000; cou_Index++)
				{
					//Pick one at random.
					int tmp_Ran = (rand() % tmp_Top);

					double tmp_Charge = NT4_Core.get_Output_Charge("Raw", tmp_Ran);

					if (tmp_Charge == tmp_Highest_Charge) 
					{ 
						//std::cout << "\n Token[" << tmp_Ran << "] {" << tmp_Charge << "} - <";

						if ((rand() % 3) == 0)
						{
							tmp_Input = "";
							tmp_Input = NT4_Core.get_Output_Pattern("Raw", tmp_Ran);
						}
						//std::cout << tmp_Input << "> ";
					}
					if (tmp_Charge > tmp_Highest_Charge) 
					{ 
						tmp_Highest_Charge = tmp_Charge; 

						//std::cout << "\n Token[" << tmp_Ran << "] {" << tmp_Charge << "} - <";
						tmp_Input = "";
						tmp_Input = NT4_Core.get_Output_Pattern("Raw", tmp_Ran);
						//std::cout << tmp_Input << "> ";
					}
				}
				std::cout << tmp_Input << " ";

				tmp_Output_Text += tmp_Input;
				tmp_Output_Text += " ";

				//std::cout << "\n ";
				NT4_Core.set_Input("Raw", tmp_Input);
				NT4_Core.query("Raw");

				shift_Input();

				Input_Tokens[block_Size - 1] = NT4_Core.get_Treetop_NID("Raw");
			}

			std::cout << "\n\n\n[" << tmp_Seed_Text << "]";
			std::cout << "[" << tmp_Output_Text << "]";
		}
		if (tmp_Chrono != NULL) { delete[] tmp_Chrono; tmp_Chrono = NULL; }

		return 1;
	}

	int output_Text()
	{
		std::cout << "\n Output Text: " << Output;

		return 1;
	}

	int train_File()
	{
		std::string tmp_Input = "";
		std::cout << "\n Enter Filename in ./GPT/ to train:";

		std::cin >> tmp_Input;
		//std::getline(std::cin, tmp_Input);

		train_Textfile(tmp_Input);

		return 1;
	}

	int run()
	{

		std::string tmp_Input = "";

		NT4_Core.load(Save_FName);

		while (tmp_Input != "/exit/")
		{
			std::cout << "\n\n";
			std::cout << "\n /save/ - Saves the current network, use after training to load updated network on startup.";
			std::cout << "\n /train/ - Choose a file to train on in ./GPT/";
			std::cout << "\n /eval/ - Go to generation mode.";
			std::cout << "\n /config/ - Update the hyperparameters.";
			std::cout << "\n /exit/ - Exit";
			std::cout << "\n\n-> Enter Option:";
			//std::getline(std::cin, tmp_Input);
			std::cin >> tmp_Input;

			if (tmp_Input == "/save/")
			{
				NT4_Core.save(Save_FName);
				continue;
			}
			if (tmp_Input == "/train/")
			{
				train_File();
				continue;
			}
			if (tmp_Input == "/config/")
			{
				std::cout << "\n Enter Construct to update: ";
				std::cin >> tmp_Input;
				NT4_Core.update_Config(tmp_Input);
				continue;
			}

			if (tmp_Input == "/eval/")
			{
				evaluate_Text();
				continue;
			}
		}
		return 1;
	}
};