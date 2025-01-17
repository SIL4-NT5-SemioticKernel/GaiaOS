class c_GPT_5
{
public:

	NT4::c_Time_Series_Generator_Module TSG_Core;

	NT4::c_Construct_API NT4_Core;

	int Block_Size;

	c_GPT_5()
	{
		NT4_Core.register_Construct("Many_To_One", "Word");

		NT4_Core.register_Construct("Many_To_One", "Ass");

		NT4_Core.create_Construct_Connection("Word", "Ass");

		TSG_Core.init(3, 1, 1);

		Block_Size = 10;
	}

	int get_Token_Count(std::string p_FName)
	{
		std::ifstream IF(p_FName);

		if (!IF.is_open())
		{
			std::cerr << "\n   V(0.0)v ERROR: File not found: " << p_FName;
		}
		else
		{
			std::cout << "\n File Opened: " << p_FName;
		}

		std::string tmp_Input = "";
		int tmp_Token_Count = 0;

		//Get the length of the file in tokens.
		while (!(IF.eof()))
		{
			tmp_Input = "";
			IF >> tmp_Input;
			tmp_Token_Count++;
		}
		IF.close();

		return tmp_Token_Count;
	}

	int train()
	{
		//For each token encode it into the raw, then into the chrono. Finally after all tokens are encoded then do the association encoding.

		//==-- Setup stuff.
		//The filename used to train on. Gathered from the user.
		std::string tmp_FName = "";

		//The meat n taters.

		//First, get the textfile to train upon. Found in ./GPT/
		std::cout << "\n Enter .txt to train upon: ";
		std::cin >> tmp_FName;

		tmp_FName = "./GPT/" + tmp_FName;

		std::string tmp_Input = "";
		int tmp_Check = 1000;
		int tmp_Current = 0;
		int tmp_Chrono_Current = 0;
		int tmp_Token_Count = get_Token_Count(tmp_FName);

		//Trained size it +1 over eval size.
		int tmp_Train_Size = Block_Size;

		uint64_t* tmp_Association_Array = NULL;
		tmp_Association_Array = new uint64_t[tmp_Train_Size];
		for (int cou_Index = 0; cou_Index < tmp_Train_Size; cou_Index++)
		{
			tmp_Association_Array[cou_Index] = 0;
		}

		std::ifstream IF(tmp_FName);

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

				tmp_Input = "";
				IF >> tmp_Input;

				if (tmp_Input != "")
				{
					NT4_Core.set_Input("Word", tmp_Input);

					NT4_Core.encode("Word");

					uint64_t tmp_Input_Treetop = NT4_Core.get_Treetop_NID("Word");

					TSG_Core.shift_Data(0);
					
					TSG_Core.set_Input_Index(0, 0, tmp_Input_Treetop);

					if (tmp_Chrono_Current > tmp_Train_Size)
					{
						TSG_Core.encode(0);
					}

					TSG_Core.output_IO(0);
					TSG_Core.NT4_Core.output_Scaffold("Raw_0");

					tmp_Chrono_Current++;
				}
			}
		}

		return 1;
	}

	void eval()
	{
		std::string tmp_Input;

		std::cout << "\n\n Enter token: ";

		while (1)
		{
			tmp_Input = "";
			std::cin >> tmp_Input;

			NT4_Core.set_Input("Word", tmp_Input);

			NT4_Core.encode("Word");

			uint64_t tmp_Input_Treetop = NT4_Core.get_Treetop_NID("Word");

			TSG_Core.shift_Data(0);

			TSG_Core.set_Input_Index(0, 0, tmp_Input_Treetop);

			TSG_Core.query_Arrays(0);

			TSG_Core.output_Bulk(0);
		}
	}

	int run()
	{
		train();

		eval();

		return 1;
	}
};
















































class c_GPT_4
{
public:

	NT4::c_Construct_API NT4_Core;

	std::vector<uint64_t> Input_Char_Block;

	int Block_Size;

	c_GPT_4()
	{
		NT4_Core.register_Construct("Many_To_One", "Word");

		NT4_Core.register_Construct("Many_To_One", "Ass");

		NT4_Core.create_Construct_Connection("Word", "Ass");

		Block_Size = 10;
	}

	void shift_Input_Char_Block()
	{
		for (int cou_Index = 0; cou_Index < (Input_Char_Block.size() - 1); cou_Index++)
		{
			Input_Char_Block[cou_Index] = Input_Char_Block[cou_Index + 1];
		}
		Input_Char_Block[Input_Char_Block.size() - 1] = 0;
	}

	int run()
	{
		std::string tmp_Input = "";

		uint64_t tmp_Association_Array[2] = { 0, 0 };

		NT4_Core.set_Input_uint("Ass", 2, tmp_Association_Array);

		Input_Char_Block.resize(Block_Size);

		int tmp_Tick = 0;

		while (1)
		{
			std::cout << "\n ->";
			std::cin >> tmp_Input;
			tmp_Input = " " + tmp_Input;

			for (int cou_Char = 0; cou_Char < tmp_Input.size(); cou_Char++)
			{
				//---std::cout << "\n shift_Input_Char_Block";
				shift_Input_Char_Block();

				//---std::cout << "\n Input_Char_Block";
				Input_Char_Block[Input_Char_Block.size() - 1] = uint64_t(tmp_Input[cou_Char]);


				//---std::cout << "\n set_Input_uint";
				NT4_Core.set_Input_uint("Word", Input_Char_Block.size(),  Input_Char_Block.data());

				tmp_Association_Array[0] = tmp_Association_Array[1];
				tmp_Association_Array[1] = NT4_Core.get_Treetop_NID("Word");

				if (tmp_Tick > Block_Size)
				{
					/*-*/std::cout << "\n encode Word";
					NT4_Core.encode("Word");

					/*-*/std::cout << "\n output_Scaffold";
					NT4_Core.output_Scaffold("Word");

					/*-*/std::cout << "\n encode Association";
					NT4_Core.set_Input_uint("Ass", 2, tmp_Association_Array);
					NT4_Core.encode("Ass");
					NT4_Core.output_Scaffold("Ass");

					generate_Output();
				}


				tmp_Tick++;
			}



		}
		return 1;
	}

	void generate_Output(int p_Ticks = 100)
	{
		uint64_t tmp_Association_Array[2] = { 0, 0 };

		NT4_Core.set_Input_uint("Ass", 2, tmp_Association_Array);

		std::vector<uint64_t> tmp_Input_Char_Block = Input_Char_Block;
		std::cout << "\n\n<[" << tmp_Input_Char_Block.size() << "]";
		for (int cou_Char = 0; cou_Char < tmp_Input_Char_Block.size(); cou_Char++)
		{
			std::cout << tmp_Input_Char_Block[cou_Char] << " ";
		}

		for (int cou_Tick = 0; cou_Tick < p_Ticks; cou_Tick++)
		{
			/*-*/std::cout << "\n query Word";
			NT4_Core.set_Input_uint("Word", Block_Size, tmp_Input_Char_Block.data());
			NT4_Core.reset_Output("Word");
			NT4_Core.query_Spacial("Word");
			NT4_Core.gather_Treetops("Word");
			NT4_Core.output_Output("Word");

			//tmp_Association_Array[0] = tmp_Association_Array[1];
			//tmp_Association_Array[1] = NT4_Core.get_Treetop_NID("Word");

			for (int cou_O = 0; cou_O < NT4_Core.get_Output_Depth("Word"); cou_O++)
			{
				tmp_Association_Array[0] = NT4_Core.get_Output_Treetop_NID("Word", cou_O);
				tmp_Association_Array[1] = 0;

				NT4_Core.reset_Output("Ass");
				NT4_Core.set_Input_uint("Ass", 1, tmp_Association_Array);
				NT4_Core.query_Spacial("Ass");
			}

			/*-*/std::cout << "\n query Ass";
			NT4_Core.gather_Treetops("Ass");
			NT4_Core.output_Output_uint("Ass");
			//Get the associations and score them.
			for (int cou_A = 0; cou_A < NT4_Core.get_Output_Depth("Ass"); cou_A++)
			{
				//For each association check all the current nodes in the input context window.
				//for (int cou_Nodes=0;cou_Nodes<)

			}

			NT4_Core.reset_Output("Word");
			NT4_Core.pull_From_Upper_Index("Word", "Ass", 1);
			NT4_Core.output_Output("Word");

			double tmp_Highest = 0;
			u_Data tmp_Current_Char;
			std::vector<uint64_t> tmp_Out;
			for (int cou_WO = 0; cou_WO < NT4_Core.get_Output_Depth("Word"); cou_WO++)
			{
				double tmp_Charge = NT4_Core.get_Output_Charge("Word", cou_WO);
				tmp_Out = NT4_Core.get_Output_Pattern_uint("Word", cou_WO);
				if (tmp_Charge > tmp_Highest)
				{
					tmp_Highest = tmp_Charge;
					tmp_Current_Char.U = tmp_Out[Block_Size - 1];
				}
			}

			for (int cou_Index = 0; cou_Index < (tmp_Input_Char_Block.size() - 1); cou_Index++)
			{
				tmp_Input_Char_Block[cou_Index] = tmp_Input_Char_Block[cou_Index + 1];
			}

			tmp_Input_Char_Block[tmp_Input_Char_Block.size() - 1] = tmp_Current_Char.U;
			/*
			for (int cou_Tick = 0; cou_Tick < p_Ticks; cou_Tick++)
			{
				tmp_Input_Char_Block[tmp_Input_Char_Block.size() - 1] = 0;
			}*/
		}



		std::cout << "\n>\n";
	}
};

class c_GPT_3
{
public:

	//The length of tokens to read in at a time.
	int block_Size;

	c_Construct_API NT4_Core;

	std::string Save_FName;

	uint64_t* Chrono_Inputs;
	int Chrono_Input_Depth;

	std::vector<uint64_t> Context;

	uint64_t Association_Inputs[2];

	c_GPT_3(const int p_Block_Size = 100)
	{
		block_Size = p_Block_Size;

		Chrono_Inputs = NULL;
		Chrono_Input_Depth = 0;

		Save_FName = "./cores/GPT.ssv";
	}

	//Load the save file.
	void load()
	{
		NT4_Core.load(Save_FName);
	}

	void initialize_Network_Constructs()
	{
		//Try to load.
		load();

		//If no load then new.
		if (NT4_Core.get_Construct_Count() == 0)
		{
			NT4_Core.register_Construct("Many_To_One", "Chrono");
			NT4_Core.register_Construct("Many_To_One", "Ass"); //Associations
			NT4_Core.register_Construct("Many_To_One", "Raw");
			NT4_Core.create_Construct_Connection("Raw", "Chrono");
			NT4_Core.create_Construct_Connection("Raw", "Ass");

			//This is a bit hacky, but atm we set the charing tier of the raw way above where it is reasonable.
			//This causes it to not actually charge, but it does fill the scaffold NULLCAN style with no reinforcement.
			NT4_Core.set_Charging_Tier("Raw", 100);
			NT4_Core.set_Action_Potential_Threshold("Chrono", 0.5);
			NT4_Core.set_Action_Potential_Threshold("Ass", 0.5);
			NT4_Core.set_Modifier_Charge("Ass", 0.9);
			NT4_Core.set_Base_Charge("Chrono", 1000);
			NT4_Core.set_Action_Potential_Threshold("Chrono", 0.1);
			NT4_Core.save_Config("Chrono");
			NT4_Core.save_Config("Ass");
			NT4_Core.save_Config("Raw");
		}
	}

	int get_Token_Count(std::string p_FName)
	{
		std::ifstream IF(p_FName);

		if (!IF.is_open())
		{
			std::cerr << "\n   V(0.0)v ERROR: File not found: " << p_FName;
		}
		else
		{
			std::cout << "\n File Opened: " << p_FName;
		}

		std::string tmp_Input = "";
		int tmp_Token_Count = 0;

		//Get the length of the file in tokens.
		while (!(IF.eof()))
		{
			tmp_Input = "";
			IF >> tmp_Input;
			tmp_Token_Count++;
		}
		IF.close();

		return tmp_Token_Count;
	}

	//This basically just allocates the Chrono constructs input array.
	void allocate_Chrono(const int p_Chrono_Input_Depth)
	{

		Chrono_Input_Depth = p_Chrono_Input_Depth;

		if (Chrono_Inputs != NULL) { delete Chrono_Inputs; Chrono_Inputs = NULL; }

		//We need to set the chrono input up to the correct depth.
		Chrono_Inputs = new uint64_t[Chrono_Input_Depth];
		for (int cou_Index = 0; cou_Index < Chrono_Input_Depth; cou_Index++)
		{
			Chrono_Inputs[cou_Index] = 0;
		}
		NT4_Core.set_Input_uint("Chrono", Chrono_Input_Depth, Chrono_Inputs);

		delete[] Chrono_Inputs; Chrono_Inputs = NULL;
	}

	int train()
	{
		//For each token encode it into the raw, then into the chrono. Finally after all tokens are encoded then do the association encoding.

		//==-- Setup stuff.
		//The filename used to train on. Gathered from the user.
		std::string tmp_FName = "";

		allocate_Chrono(block_Size);

		//The meat n taters.

		//First, get the textfile to train upon. Found in ./GPT/
		std::cout << "\n Enter .txt to train upon: ";
		std::cin >> tmp_FName;

		tmp_FName = "./GPT/" + tmp_FName;

		std::string tmp_Input = "";
		int tmp_Check = 1000;
		int tmp_Current = 0;
		int tmp_Chrono_Current = 0;
		int tmp_Token_Count = get_Token_Count(tmp_FName);

		//Trained size it +1 over eval size.
		int tmp_Train_Size = block_Size + 1;

		uint64_t* tmp_Association_Array = NULL;
		tmp_Association_Array = new uint64_t[tmp_Train_Size];
		for (int cou_Index = 0; cou_Index < tmp_Train_Size; cou_Index++)
		{
			tmp_Association_Array[cou_Index] = 0;
		}

		std::ifstream IF(tmp_FName);

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

				tmp_Input = "";
				IF >> tmp_Input;

				if (tmp_Input != "")
				{
					NT4_Core.set_Input("Raw", tmp_Input);

					NT4_Core.encode("Raw");

					NT4_Core.pull_Chrono_From_Lower_Connection("Chrono");

					if (tmp_Chrono_Current > tmp_Train_Size)
					{
						NT4_Core.encode("Chrono");
					}
					tmp_Chrono_Current++;
				}
			}
		}

		return 1;
	}

	int eval()
	{
		NT4_Core.update_Config("Raw");
		NT4_Core.update_Config("Chrono");
		NT4_Core.update_Config("Ass");

		allocate_Chrono(block_Size);

		int tmp_Tokens_To_Gen = 1024;

		std::string tmp_Input = "";

		while (tmp_Input != "/exit/")
		{
			//This requires building up the prompt into the context. Then doing a step by step walk of a block size chunk on the end to generate each token sequentially. Remember, training data is +1.
			std::cout << "\n Enter /end/ when done with your prompt: ";
			Context.clear();
			tmp_Input = "";
			while(tmp_Input != "/end/")
			{
				//Build up the current context.
				tmp_Input = "";
				//std::cout << "\n<" << Context.size() << ">";
				std::cin >> tmp_Input;

				if (tmp_Input == "/end/") { continue; }

				NT4_Core.set_Input("Raw", tmp_Input);
				NT4_Core.encode("Raw");

				Context.push_back(NT4_Core.get_Treetop_NID("Raw"));

				if (tmp_Input == "/exit/") { return 1; }
			}

			//Now that the context is gathered we can start iterating the block within the every growing context. We just set the input of Chrono to the Context with the offset of (Context.size() - block_Size) and a depth of block_Size. 
			//With the block_Size +1 encoding of the training data this means we expect a trace output with a depth +1 over the current Input.
			//We use this extra token as the next one in the series.
			for (int cou_Token = 0; cou_Token < tmp_Tokens_To_Gen; cou_Token++)
			{
				NT4_Core.update_Config("Chrono");
				if (!(cou_Token % 10)) { std::cout << "\n"; }
				//---std::cout << " ... ";
				//---std::system("PAUSE");
				//The offset calculates the block position within the context.
				// [context [block][desired_token]]
				// offset = context - block_Size
				//int tmp_Offset = Context.size()- block_Size;

				//Desired index in relation to the context.
				//One further than the current context array.
				int tmp_Desired = Context.size();

				//The current start position.
				int tmp_Start = tmp_Desired - (2 * (block_Size - 1));
				if (tmp_Start < 0) { tmp_Start = 0; }

				//The current AW block start position.
				int tmp_AW_Start = Context.size() - block_Size;

				//2(block size - 1) w/ the start offset == tmp_Desired - tmp_Start
				int tmp_Current_Field = tmp_Desired - tmp_Start;

				//---std::cout << "\n\n\n Token[" << cou_Token << " / " << tmp_Tokens_To_Gen << "] - Start = " << tmp_Start << " - Desired = " << tmp_Desired << " - Field: " << tmp_Current_Field;

				//Start is twice 

				std::vector<double> tmp_Attention_Weights(block_Size);
				std::vector<double> tmp_Attention_Weights_Count(block_Size);
				std::vector<double> tmp_Attention_Weights_Rel(block_Size);
				std::vector<double> tmp_Attention_Weights_Final(block_Size);

				//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 0";
				for (int cou_A = 0; cou_A < block_Size; cou_A++)
				{
					tmp_Attention_Weights[cou_A] = 0;
					tmp_Attention_Weights_Count[cou_A] = 0;
					tmp_Attention_Weights_Rel[cou_A] = 0;
					tmp_Attention_Weights_Final[cou_A] = 0;
				}

				//So we loop through each node in the current block.
				// Then for each node we do a leg specified charge with the current node at [0] in the Chrono to get relationships to the future nodes relative to the current.
				//  For every output for each nodes we iterate through matching them to future nodes and tracking the occurances and RCs found.
				//  We then go back through and compute the scores in AW
				//  With scores computed we walk through each output again
				//   If the output matches the current dendrite for it's position then it is updated.
				// Then the final Chrono charge happens and the new node it gathered.
				//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 1";
				for (int cou_Node = tmp_Start; cou_Node < Context.size(); cou_Node++)
				{
					//---std::cerr << "\n [" << cou_Node << "] " << tmp_Start << " -- " << Context.size();
					//---std::cout << "\n\n\nCurrent Block Node[" << cou_Node << "] " << Context[cou_Node];
					//---NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_Node]);

					//Search the Chrono for forward associations.
					//Take the current node, query the association array.
					NT4_Core.reset_Output("Chrono");
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 a";
					NT4_Core.set_Input_uint("Chrono", 1, &(Context[cou_Node]));
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 b";
					NT4_Core.query_Given_Index("Chrono", 0);
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 c";
					NT4_Core.gather_Treetops("Chrono");
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 d";
					//NT4_Core.output_Input_uint("Chrono");
					//NT4_Core.output_Output_uint("Chrono");
					//NT4_Core.output_Scaffold("Chrono");

					std::vector<uint64_t> tmp_Token;

					double tmp_Pattern_Score;

					//Loop through each output and tally up the scores based on matching legs.
					for (int cou_T = 0; cou_T < NT4_Core.get_Output_Depth("Chrono"); cou_T++)
					{
						tmp_Token.clear();
						tmp_Token = NT4_Core.get_Output_Pattern_uint("Chrono", cou_T);

						//Tally the occurances in the entire pattern to add to the relavancy factor;
						tmp_Pattern_Score = 0.0;

						//Now step through each leg of the token started at 1 skipping the 0th index.
						for (int cou_Step = 1; cou_Step < tmp_Token.size(); cou_Step++)
						{

							//---std::cout << "\n Step: " << cou_Step << " cou_Node: " << cou_Node;;
							//Check if the current step is beyond the end of the current field.
							if ((cou_Step + cou_Node) < Context.size())
							{
								//Check if the current node is less than the current start of the attention weights.
								if ((cou_Step + cou_Node) > tmp_AW_Start)
								{
									if (Context[cou_Step + cou_Node] == tmp_Token[cou_Step])
									{
										tmp_Pattern_Score++;

										if (cou_Node >= tmp_AW_Start)
										{
											//tmp_Attention_Weights[cou_Node - tmp_AW_Start] += NT4_Core.get_Output_RC("Chrono", cou_T);
											//tmp_Attention_Weights_Count[cou_Node - tmp_AW_Start]++;
										}
									}
								}
							}
							//---std::cout << "\n Step: " << cou_Step << " cou_Node: " << cou_Node;;
							//Check if the current step is beyond the end of the current field.
							if ((cou_Step + cou_Node) < Context.size())
							{
								//Check if the current node is less than the current start of the attention weights.
								if ((cou_Step + cou_Node) > tmp_AW_Start)
								{
									if (Context[cou_Step + cou_Node] == tmp_Token[cou_Step])
									{
										//tmp_Attention_Weights_Rel[(cou_Node + cou_Step) - tmp_AW_Start] += tmp_Pattern_Score;// / double(tmp_Token.size());

										//---std::cout << " -= Match =- " << ((cou_Node + cou_Step) - tmp_AW_Start);
										//tmp_Attention_Weights[(cou_Node + cou_Step) - tmp_AW_Start] += NT4_Core.get_Output_RC("Chrono", cou_T);
										tmp_Attention_Weights[(cou_Node + cou_Step) - tmp_AW_Start] += (NT4_Core.get_Output_RC("Chrono", cou_T) / tmp_Pattern_Score);
										tmp_Attention_Weights_Count[(cou_Node + cou_Step) - tmp_AW_Start]++;
									}
								}
							}
						}
					}
				}

				//---std::cerr << "\n ------------------------------ 0";
				
				double tmp_Highest = 0.0;
				double tmp_Lowest = 99999999;
				 
				//Now that we have all the attention weights we can setup the weighted query to get the final output token prediction set.
				for (int cou_W = 0; cou_W < block_Size; cou_W++)
				{
					//tmp_Attention_Weights_Final[cou_W] = ((tmp_Attention_Weights_Count[cou_W]) / (tmp_Attention_Weights[cou_W] + 0.01)) + tmp_Attention_Weights_Rel[cou_W];
					//tmp_Attention_Weights_Final[cou_W] = ((tmp_Attention_Weights_Count[cou_W] + 1) / (tmp_Attention_Weights[cou_W] + 0.01));
					tmp_Attention_Weights_Final[cou_W] = ((((tmp_Attention_Weights_Count[cou_W]) + (tmp_Attention_Weights[cou_W]) + (tmp_Attention_Weights_Rel[cou_W])) / 3) + 1);
					if (tmp_Attention_Weights_Final[cou_W] > tmp_Highest) { tmp_Highest = tmp_Attention_Weights_Final[cou_W]; }
					if ((tmp_Attention_Weights_Final[cou_W] < tmp_Lowest) && (tmp_Attention_Weights_Final[cou_W] != 0)) { tmp_Lowest = tmp_Attention_Weights_Final[cou_W]; }
				}
				if (tmp_Lowest > tmp_Highest) { tmp_Lowest = 0; }

				for (int cou_W = 0; cou_W < block_Size; cou_W++)
				{
					if (tmp_Attention_Weights_Final[cou_W] == 0) { continue; }
					if ((tmp_Highest - tmp_Lowest) == 0) { continue; }
					tmp_Attention_Weights_Final[cou_W] = ((tmp_Attention_Weights_Final[cou_W] - tmp_Lowest) / (tmp_Highest - tmp_Lowest)) * NT4_Core.get_Base_Charge("Chrono") + (NT4_Core.get_Base_Charge("Chrono") / 100);
				}
				
				/*
				//std::cout << "\n Highest: " << tmp_Highest << " Lowest: " << tmp_Lowest << " Diff: " << (tmp_Highest - tmp_Lowest);
				for (int cou_W = 0; cou_W < block_Size; cou_W++)
				{
					//std::cout << "\n Context.size(): " << Context.size() << " cou_W: " << cou_W << " Con-block_Size + cou_W: " << ((Context.size() - block_Size) + cou_W);
					std::cout << "\n AW[" << cou_W << "] <";
					//NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_W + p_Offset]);
					if ((((Context.size() - block_Size) + cou_W) < (Context.size())) && (((Context.size() - block_Size) + cou_W) >= 0)){ NT4_Core.bp_O(Context[((Context.size() - block_Size) + cou_W)]); }
					//std::cout << " ~~~  Occurance Weight: " << tmp_Attention_Weights_Count[cou_W] << " RC Weight: " << tmp_Attention_Weights[cou_W];
					//std::cout << " Rel: " << tmp_Attention_Weights_Rel[cou_W];
					//std::cout << " Final Score (Occ / RC): " << tmp_Attention_Weights_Final[cou_W];
					std::cout << " - " << tmp_Attention_Weights_Final[cou_W] << ">";
				}

				*/
				
				//---std::cerr << "\n ------------------------------ 2";
				NT4_Core.reset_Output("Chrono");
				//---std::cerr << "\n ------------------------------ 3";

				//---std::cerr << "\n cou_Token[" << Context.size() << "] block_Size: " << block_Size << " Context.size(): " << Context.size();
				//---std::cerr << " (tmp_Desired - block_Size - 1):" << (tmp_Desired - block_Size - 1);
				if (Context.size() > block_Size)
				{
					NT4_Core.set_Input_Charging_Mask("Chrono", tmp_Attention_Weights_Final);
					//---std::cerr << "\n ------------------------------ ";
					//NT4_Core.set_Input_uint("Chrono", (block_Size - 1), &(Context[tmp_Desired - block_Size]));
					NT4_Core.set_Input_uint("Chrono", (block_Size), &(Context[tmp_Desired - block_Size]));
					//---std::cerr << "\n ------------------------------ 4";
					NT4_Core.query_Spacial("Chrono");

					NT4_Core.gather_Treetops("Chrono");
				}
				else
				{
					std::vector<uint64_t> tmp_Block(block_Size);

					for (int cou_B = 0; cou_B < Context.size(); cou_B++)
					{

						//std::cerr << "\n      (Context.size() - 1) - (block_Size - cou_B): " << int((Context.size() - 1) - (block_Size - cou_B)) << " cou_B: " << cou_B;
						//std::cerr << "\n      (Context.size() - 1) - (block_Size - cou_B): " << int((block_Size - (Context.size() - 1) + cou_B)) << " cou_B: " << cou_B;
						//std::cerr << "\n      ((block_Size - Context.size() + couB): " << int((block_Size - Context.size() + cou_B)) << " cou_B: " << cou_B;

						tmp_Block[(block_Size - Context.size() + cou_B)] = Context[cou_B];
					}

					NT4_Core.set_Input_uint("Chrono", block_Size, &(tmp_Block[0]));
					//---std::cerr << "\n ------------------------------ 4";
					NT4_Core.query_Spacial("Chrono");
					std::cout << "_";

					NT4_Core.gather_Treetops("Chrono");

					if (NT4_Core.get_Output_Depth("Chrono") == 0)
					{
						std::cout << "&";
						NT4_Core.reset_Output("Chrono");
						NT4_Core.query("Chrono");
						NT4_Core.gather_Treetops("Chrono");
					}
				}
				//---NT4_Core.output_Input("Chrono");
				//NT4_Core.set_Input_uint("Chrono", 5, Context_Tokens);
				//NT4_Core.query("Chrono");

				//---std::cerr << "\n ------------------------------ 5";
				//NT4_Core.output_Output_uint("Chrono");


				std::vector<uint64_t> tmp_Final_Vector;
				std::vector<uint64_t> tmp_Final_Out;

				tmp_Final_Out;
				//---std::cout << "\n Generated Final Token Pool:";
				//---std::cerr << "\n ------------------------------ 6";
				for (int cou_O = 0; cou_O < NT4_Core.get_Output_Depth("Chrono"); cou_O++)
				{
					//---std::cout << "\n Output[" << cou_O << "] ";
					tmp_Final_Out = NT4_Core.get_Output_Pattern_uint("Chrono", cou_O);

					//---NT4_Core.bp_O(tmp_Final_Out[tmp_Final_Out.size() - 1]);
					
					//---std::cout << " {" << NT4_Core.get_Output_Charge("Chrono", cou_O) << "}";

					auto tmp_Unique = std::find(tmp_Final_Vector.begin(), tmp_Final_Vector.end(), tmp_Final_Out[tmp_Final_Out.size() - 1]);

					// Check if the value was found
					if (tmp_Unique != tmp_Final_Vector.end())
					{
						//std::cout << "Value " << valueToFind << " found at index " << std::distance(tmp_Final_Vector.begin(), tmp_Unique) << std::endl;
					}
					else 
					{
						//std::cout << "Value " << valueToFind << " not found in the vector" << std::endl;
						tmp_Final_Vector.push_back(tmp_Final_Out[tmp_Final_Out.size() - 1]);
					}
				}

				//---std::cerr << "\n ------------------------------ 7";

				int tmp_Ran = 0;
				if (tmp_Final_Vector.size() > 0)
				{
					//tmp_Output_Spread.output_Tree();
					tmp_Ran = rand() % tmp_Final_Vector.size();
					
					/*
					while (tmp_Final_Vector[tmp_Ran] != (Context[Context.size() - 1]))
					{
						tmp_Ran = rand() % tmp_Final_Vector.size();
					}*/

					//std::cout << "\n\n Final Token Chosen: ";
					std::cout << " ";
					NT4_Core.bp_O(tmp_Final_Vector[tmp_Ran]);

					Context.push_back(tmp_Final_Vector[tmp_Ran]);
				}
				else
				{
					Context.push_back(0);
				}

				//---std::cerr << "\n ------------------------------ 10";
				
			}
		}


		return 1;
	}

	int run()
	{

		std::string tmp_Input = "";

		initialize_Network_Constructs();

		while (tmp_Input != "/exit/")
		{
			std::cout << "\n\n";
			std::cout << "\n /save/ - Saves the current network, use after training to load updated network on startup.";
			std::cout << "\n /train/ - Choose a file to train on in ./GPT/";
			std::cout << "\n /eval/ - Go to generation mode.";
			std::cout << "\n /config/ - Update the hyperparameters.";
			std::cout << "\n /set_BS/ - Set the block_Size and the context_Size. This is separate from the ML hyperparams.";
			std::cout << "\n /exit/ - Exit";
			std::cout << "\n\n-> Enter Option:";
			//std::getline(std::cin, tmp_Input);
			std::cin >> tmp_Input;

			if (tmp_Input == "/save/")
			{
				std::cout << "\n Filename to save core as (saved in the dir ./cores/) (no spaces in filename): ";
				std::cin >> Save_FName;
				Save_FName = "./cores/" + Save_FName;
				NT4_Core.save(Save_FName);
				continue;
			}
			if (tmp_Input == "/load/")
			{
				NT4_Core.load(Save_FName);
				continue;
			}
			if (tmp_Input == "/train/")
			{
				train();
				continue;
			}
			if (tmp_Input == "/config/")
			{
				continue;
			}
			if (tmp_Input == "/set_BS/")
			{
				int tmp_Int = 0;
				std::cout << "\n Current block_Size: " << block_Size;
				std::cin >> block_Size;
			}

			if (tmp_Input == "/eval/")
			{
				eval();
				continue;
			}
		}
		return 1;
	}
};
























































class c_GPT_2
{
public:

	//The length of tokens to read in at a time.
	int block_Size;

	c_Construct_API NT4_Core;

	std::string Save_FName;

	uint64_t* Chrono_Inputs;
	int Chrono_Input_Depth;

	std::vector<uint64_t> Context;

	uint64_t Association_Inputs[2];

	c_GPT_2(const int p_Block_Size = 5)
	{
		block_Size = p_Block_Size;

		Chrono_Inputs = NULL;
		Chrono_Input_Depth = 0;

		Save_FName = "./cores/GPT.ssv";
	}

	//Load the save file.
	void load()
	{
		NT4_Core.load(Save_FName);
	}

	void initialize_Network_Constructs()
	{
		//Try to load.
		load();

		//If no load then new.
		if (NT4_Core.get_Construct_Count() == 0)
		{
			NT4_Core.register_Construct("Many_To_One", "Chrono");
			NT4_Core.register_Construct("Many_To_One", "Ass"); //Associations
			NT4_Core.register_Construct("Many_To_One", "Raw");
			NT4_Core.create_Construct_Connection("Raw", "Chrono");
			NT4_Core.create_Construct_Connection("Raw", "Ass");

			//This is a bit hacky, but atm we set the charing tier of the raw way above where it is reasonable.
			//This causes it to not actually charge, but it does fill the scaffold NULLCAN style with no reinforcement.
			NT4_Core.set_Charging_Tier("Raw", 100);
			NT4_Core.set_Action_Potential_Threshold("Chrono", 0.5);
			NT4_Core.set_Action_Potential_Threshold("Ass", 0.5);
			NT4_Core.set_Modifier_Charge("Ass", 0.9);
			NT4_Core.set_Action_Potential_Threshold("Chrono", 0.5);
		}
	}

	int get_Token_Count(std::string p_FName)
	{
		std::ifstream IF(p_FName);

		if (!IF.is_open())
		{
			std::cerr << "\n   V(0.0)v ERROR: File not found: " << p_FName;
		}
		else
		{
			std::cout << "\n File Opened: " << p_FName;
		}

		std::string tmp_Input = "";
		int tmp_Token_Count = 0;

		//Get the length of the file in tokens.
		while (!(IF.eof()))
		{
			tmp_Input = "";
			IF >> tmp_Input;
			tmp_Token_Count++;
		}
		IF.close();

		return tmp_Token_Count;
	}

	//This basically just allocates the Chrono constructs input array.
	void allocate_Chrono(const int p_Chrono_Input_Depth)
	{

		Chrono_Input_Depth = p_Chrono_Input_Depth;

		if (Chrono_Inputs != NULL) { delete Chrono_Inputs; Chrono_Inputs = NULL; }

		//We need to set the chrono input up to the correct depth.
		Chrono_Inputs = new uint64_t[Chrono_Input_Depth];
		for (int cou_Index = 0; cou_Index < Chrono_Input_Depth; cou_Index++)
		{
			Chrono_Inputs[cou_Index] = 0;
		}
		NT4_Core.set_Input_uint("Chrono", Chrono_Input_Depth, Chrono_Inputs);

		delete[] Chrono_Inputs;
	}

	int train()
	{
		//For each token encode it into the raw, then into the chrono. Finally after all tokens are encoded then do the association encoding.

		//==-- Setup stuff.
		//The filename used to train on. Gathered from the user.
		std::string tmp_FName = "";

		allocate_Chrono(block_Size);

		//The meat n taters.

		//First, get the textfile to train upon. Found in ./GPT/
		std::cout << "\n Enter .txt to train upon: ";
		std::cin >> tmp_FName;

		tmp_FName = "./GPT/" + tmp_FName;

		std::string tmp_Input = "";
		int tmp_Check = 1000;
		int tmp_Current = 0;
		int tmp_Chrono_Current = 0;
		int tmp_Token_Count = get_Token_Count(tmp_FName);

		//Trained size it +1 over eval size.
		int tmp_Train_Size = block_Size + 1;

		uint64_t* tmp_Association_Array = NULL;
		tmp_Association_Array = new uint64_t[tmp_Train_Size];
		for (int cou_Index = 0; cou_Index < tmp_Train_Size; cou_Index++)
		{
			tmp_Association_Array[cou_Index] = 0;
		}

		std::ifstream IF(tmp_FName);

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

				tmp_Input = "";
				IF >> tmp_Input;

				if (tmp_Input != "")
				{
					NT4_Core.set_Input("Raw", tmp_Input);

					NT4_Core.encode("Raw");

					for (int cou_Roll = 0; cou_Roll < (tmp_Train_Size - 1); cou_Roll++)
					{
						tmp_Association_Array[cou_Roll] = tmp_Association_Array[cou_Roll + 1];
					}
					tmp_Association_Array[tmp_Train_Size - 1] = 0;
					tmp_Association_Array[tmp_Train_Size - 1] = NT4_Core.get_Treetop_NID("Raw");

					NT4_Core.pull_Chrono_From_Lower_Connection("Chrono");

					if (tmp_Chrono_Current > tmp_Train_Size)
					{
						NT4_Core.encode("Chrono");
					}

					//std::cout << "\n";
					for (int cou_Index = 0; cou_Index < (tmp_Train_Size - 1); cou_Index++)
					{
						for (int cou_Step = (cou_Index + 1); cou_Step < tmp_Train_Size; cou_Step++)
						{
							Association_Inputs[0] = tmp_Association_Array[cou_Index];
							Association_Inputs[1] = tmp_Association_Array[cou_Step];

							NT4_Core.set_Input_uint("Ass", 2, Association_Inputs);
							//NT4_Core.output_Input_uint("Ass");
							NT4_Core.encode("Ass");
							//NT4_Core.output_Scaffold("Ass");
						}
					}

					tmp_Chrono_Current++;
				}
			}
		}

		return 1;
	}

	int eval()
	{
		NT4_Core.update_Config("Raw");
		NT4_Core.update_Config("Chrono");
		NT4_Core.update_Config("Ass");

		int tmp_Tokens_To_Gen = 1024;

		std::string tmp_Input = "";

		uint64_t Context_Tokens[5];

		NT4_Core.set_Input("Raw", "ghoul");
		NT4_Core.encode("Raw");
		Context_Tokens[0] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "friend");
		NT4_Core.encode("Raw");
		Context_Tokens[1] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "Randolph");
		NT4_Core.encode("Raw");
		Context_Tokens[2] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "cat");
		NT4_Core.encode("Raw");
		Context_Tokens[3] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "snow");
		NT4_Core.encode("Raw");
		Context_Tokens[4] = NT4_Core.get_Treetop_NID("Raw");

		//For each node
			//Query associations
				//Compare each association to every node in the context
					//Any matches you ++ the attention_Counter for all tokens related in the string, once for each connection.

		while (tmp_Input != "/exit/")
		{
			//This requires building up the prompt into the context. Then doing a step by step walk of a block size chunk on the end to generate each token sequentially. Remember, training data is +1.
			std::cout << "\n Enter {" << block_Size << "} Tokens: ";
			Context.clear();
			for (int cou_P = 0; cou_P < block_Size; cou_P++)
			{
				//Build up the current context.
				tmp_Input = "";
				std::cout << "\n<" << Context.size() << ">";
				std::cin >> tmp_Input;

				NT4_Core.set_Input("Raw", tmp_Input);
				NT4_Core.encode("Raw");

				Context.push_back(NT4_Core.get_Treetop_NID("Raw"));

				if (tmp_Input == "/exit/") { return 1; }
			}

			//Now that the context is gathered we can start iterating the block within the every growing context. We just set the input of Chrono to the Context with the offset of (Context.size() - block_Size) and a depth of block_Size. 
			//With the block_Size +1 encoding of the training data this means we expect a trace output with a depth +1 over the current Input.
			//We use this extra token as the next one in the series.
			for (int cou_Token = 0; cou_Token < tmp_Tokens_To_Gen; cou_Token++)
			{
				//---std::cout << " ... ";
				//---std::system("PAUSE");
				//The offset calculates the block position within the context.
				// [context [block][desired_token]]
				// offset = context - block_Size
				int p_Offset = Context.size()- block_Size;

				std::vector<double> tmp_Attention_Weights(block_Size);
				std::vector<double> tmp_Attention_Weights_Count(block_Size);
				std::vector<double> tmp_Attention_Weights_Final(block_Size);

				//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 0";
				for (int cou_A = 0; cou_A < block_Size; cou_A++)
				{
					tmp_Attention_Weights[cou_A] = 0;
					tmp_Attention_Weights_Count[cou_A] = 0;
					tmp_Attention_Weights_Final[cou_A] = 0;
				}

				//So we loop through each node in the current block.
				//Then we take the output associations and check against every node in the entire context.
				//If a node is found in the context then the RC value is added to the nodes which fall within the block within the context.
				//If a node finds another node within the block then both have the RC value added.
				//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 1";
				for (int cou_Node = 0; cou_Node < Context.size(); cou_Node++)
				{
					//---std::cerr << "\n [" << cou_Node << "] " << p_Offset << " -- " << Context.size();
					//---std::cout << "\n\n\nCurrent Block Node[" << cou_Node << "] " << Context[cou_Node];
					//---NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_Node]);

					//Take the current node, query the association array.
					NT4_Core.reset_Output("Ass");
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 a";
					NT4_Core.set_Input_uint("Ass", 1, &(Context[cou_Node])); 
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 b";
					NT4_Core.query("Ass");
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 c";
					NT4_Core.gather_Treetops("Ass");
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++----------------- 1 d";
					//NT4_Core.output_Input_uint("Ass");
					//NT4_Core.output_Output_uint("Ass");
					//NT4_Core.output_Scaffold("Ass");

					float tmp_RC = 0;
					int tmp_Count = 0;
					std::vector<uint64_t> tmp_Token;
					bool tmp_flg_Token = 0;
					//Now to check the returned tokens against all the other tokens in the array and add the RC value up for the given token.
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 2";
					//---std::cout << "\n ~~~ " << NT4_Core.get_Output_Depth("Ass") << " outputs found. ";
					for (int cou_T = 0; cou_T < NT4_Core.get_Output_Depth("Ass"); cou_T++)
					{
						tmp_Token.clear();
						tmp_Token = NT4_Core.get_Output_Pattern_uint("Ass", cou_T);

						//---std::cout << "\n\n     Output[" << cou_T << "]";
						//---std::cout << "\n [0]";
						//---NT4_Core.output_Backpropagated_Symbol_NID(tmp_Token[0]);
						//---std::cout << "\n [1]";
						//---NT4_Core.output_Backpropagated_Symbol_NID(tmp_Token[1]);

						tmp_RC = 0;
						tmp_Count = 0;

						//If the [0] token matches the current context generating the query then set the flag to [1].
						//We want the token the current node is associated with, not the current node itself.
						tmp_flg_Token = 0;
						if (tmp_Token[0] == Context[cou_Node]) { tmp_flg_Token = 1; }
						//---std::cout << "\n               flg: " << tmp_flg_Token;

						//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 3";

						for (int cou_Check = 0; cou_Check < (Context.size()); cou_Check++)
						{
							//---std::cout << "\n Check[" << cou_Check << "]";
							//Compare to every token in the context, adding to the counter when found.
							//---std::cout << "\nContext node: ";
							//---NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_Check]);
							//---std::cout << "\n Compared to:";
							//---NT4_Core.output_Backpropagated_Symbol_NID(tmp_Token[tmp_flg_Token]);
							//---std::cout << "\n Results: " << (Context[cou_Check] == tmp_Token[tmp_flg_Token]);
							if (Context[cou_Check] == tmp_Token[tmp_flg_Token]) { tmp_RC += NT4_Core.get_Output_RC("Ass", cou_T); tmp_Count++; }
						}

						//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 4";

						if (cou_Node >= p_Offset)
						{
							//The searching node gets a boost as well.
							tmp_Attention_Weights[cou_Node - p_Offset] += tmp_RC;
							tmp_Attention_Weights_Count[cou_Node - p_Offset] += tmp_Count;
						}

						for (int cou_Check = 0; cou_Check < block_Size; cou_Check++)
						{
							//---std::cout << "\n ~!!~" << cou_Check << ": ";
							//---std::cout << " RC: " << tmp_RC << " #" << tmp_Count + 1;
							//tmp_RC = (double(tmp_RC) / (tmp_Count + 1));
							//tmp_RC = (tmp_Count);
							//---std::cout << " calc_RC: " << tmp_RC;

							//Compare to every token in the context, adding to the counter when found.
							if (Context[cou_Check + p_Offset] == tmp_Token[tmp_flg_Token]) 
							{
								//---std::cout << "\n\n Context[" << cou_Check << "]";
								//---NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_Check + p_Offset]);
								//---std::cout << "\n Compared to:";
								//---NT4_Core.output_Backpropagated_Symbol_NID(tmp_Token[tmp_flg_Token]);
								//---std::cout << "\n Results: " << (Context[cou_Check + p_Offset] == tmp_Token[tmp_flg_Token]);

								//---std::cerr << "\n cou_Check: " << cou_Check << " p_Offset: " << p_Offset << " block_Size: " << block_Size << " index: " << (cou_Check + p_Offset);
								tmp_Attention_Weights[cou_Check] += tmp_RC;
								tmp_Attention_Weights_Count[cou_Check] += tmp_Count;

								//---std::cout << "\n Attention_Weights[" << cou_Check << "] " << tmp_Attention_Weights[cou_Check];
								//---std::cout << "\n tmp_Attention_Weights_Count[" << cou_Check << "] " << tmp_Attention_Weights_Count[cou_Check];
							}
						}
						//Now go through and for every matching token within the block add the RC value to it

						//When done normalize the RC results in the tmp_Attention_Weights array.
						//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 5";
					}
					//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 6";
				}
				//---std::cerr << "\n +++++++++++++++++++++++++++++++++++ 7";

				//---std::cerr << "\n ------------------------------ 0";
				//Now that we have all the attention weights we can setup the weighted query to get the final output token prediction set.
				double tmp_Highest_Weight = 1; //No divvie zeroeth
				double tmp_Highest_Weight_Count = 1; //No divvie zeroeth
				for (int cou_W = 0; cou_W < block_Size; cou_W++)
				{
					if (tmp_Attention_Weights[cou_W] > tmp_Highest_Weight) { tmp_Highest_Weight = tmp_Attention_Weights[cou_W]; }
					if (tmp_Attention_Weights_Count[cou_W] > tmp_Highest_Weight_Count) { tmp_Highest_Weight_Count = tmp_Attention_Weights_Count[cou_W]; }
				}				

				/*
				for (int cou_W = 0; cou_W < block_Size; cou_W++)
				{
					std::cout << "\n AW_Pre-Normalized[" << cou_W << "] <";
					//NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_W + p_Offset]);
					NT4_Core.bp_O(Context[cou_W + p_Offset]);
					std::cout << " ~~~ RC Weight: " << tmp_Attention_Weights[cou_W] << " Occurance Weight: " << tmp_Attention_Weights_Count[cou_W];
				}*/

				//---std::cerr << "\n ------------------------------ 1";
				for (int cou_W = 0; cou_W < block_Size; cou_W++)
				{
					tmp_Attention_Weights[cou_W] = (((tmp_Attention_Weights[cou_W]) / (tmp_Highest_Weight)) * 100.0);
					tmp_Attention_Weights_Count[cou_W] = (((tmp_Attention_Weights_Count[cou_W]) / (tmp_Highest_Weight_Count)) * 100.0);
					tmp_Attention_Weights_Final[cou_W] = ((tmp_Attention_Weights_Count[cou_W] + 1.0) / (tmp_Attention_Weights[cou_W] + 1.0));
				}				
				
				//---for (int cou_W = 0; cou_W < block_Size; cou_W++)
				//---{
					//---std::cout << "\n AW[" << cou_W << "] <";
					//NT4_Core.output_Backpropagated_Symbol_NID(Context[cou_W + p_Offset]);
					//---NT4_Core.bp_O(Context[cou_W + p_Offset]);
					//---std::cout << " ~~~  Occurance Weight: " << tmp_Attention_Weights_Count[cou_W] << "RC Weight: " << tmp_Attention_Weights[cou_W];
					//---std::cout << " Final Score (Occ / RC): " <<  tmp_Attention_Weights_Final[cou_W];
				//---}

				//---std::cerr << "\n ------------------------------ 2";
				NT4_Core.reset_Output("Chrono");
				//---std::cerr << "\n ------------------------------ 3";
				NT4_Core.set_Input_Charging_Mask("Chrono", tmp_Attention_Weights_Final);
				//---std::cerr << "\n ------------------------------ ";
				NT4_Core.set_Input_uint("Chrono", block_Size, &(Context[p_Offset]));
				//---std::cerr << "\n ------------------------------ 4";
				NT4_Core.query_Spacial("Chrono");

				//NT4_Core.set_Input_uint("Chrono", 5, Context_Tokens);
				//NT4_Core.query("Chrono");

				//---std::cerr << "\n ------------------------------ 5";
				NT4_Core.gather_Treetops("Chrono");
				//NT4_Core.output_Output_uint("Chrono");

				std::vector<uint64_t> tmp_Final_Vector;
				std::vector<uint64_t> tmp_Final_Out;

				tmp_Final_Out;
				//---std::cout << "\n Generated Final Token Pool:";
				//---std::cerr << "\n ------------------------------ 6";
				for (int cou_O = 0; cou_O < NT4_Core.get_Output_Depth("Chrono"); cou_O++)
				{
					//---std::cout << "\n Output[" << cou_O << "] ";
					tmp_Final_Out = NT4_Core.get_Output_Pattern_uint("Chrono", cou_O);

					//---NT4_Core.bp_O(tmp_Final_Out[tmp_Final_Out_Depth - 1]);
					
					//---std::cout << " {" << NT4_Core.get_Output_Charge("Chrono", cou_O) << "}";

					auto tmp_Unique = std::find(tmp_Final_Vector.begin(), tmp_Final_Vector.end(), tmp_Final_Out[tmp_Final_Out.size() - 1]);

					// Check if the value was found
					if (tmp_Unique != tmp_Final_Vector.end())
					{
						//std::cout << "Value " << valueToFind << " found at index " << std::distance(tmp_Final_Vector.begin(), tmp_Unique) << std::endl;
					}
					else 
					{
						//std::cout << "Value " << valueToFind << " not found in the vector" << std::endl;
						tmp_Final_Vector.push_back(tmp_Final_Out[tmp_Final_Out.size() - 1]);
					}
				}

				//---std::cerr << "\n ------------------------------ 7";


				//tmp_Output_Spread.output_Tree();

				int tmp_Ran = rand() % tmp_Final_Vector.size();
				//std::cout << "\n\n Final Token Chosen: ";
				std::cout << " ";
				NT4_Core.bp_O(tmp_Final_Vector[tmp_Ran]);

				//---std::cerr << "\n ------------------------------ 8";
				Context.push_back(tmp_Final_Vector[tmp_Ran]);

				//---std::cerr << "\n ------------------------------ 10";
			}
		}


		return 1;
	}

	int run()
	{

		std::string tmp_Input = "";

		initialize_Network_Constructs();

		while (tmp_Input != "/exit/")
		{
			std::cout << "\n\n";
			std::cout << "\n /save/ - Saves the current network, use after training to load updated network on startup.";
			std::cout << "\n /train/ - Choose a file to train on in ./GPT/";
			std::cout << "\n /eval/ - Go to generation mode.";
			std::cout << "\n /config/ - Update the hyperparameters.";
			std::cout << "\n /set_BS/ - Set the block_Size and the context_Size. This is separate from the ML hyperparams.";
			std::cout << "\n /exit/ - Exit";
			std::cout << "\n\n-> Enter Option:";
			//std::getline(std::cin, tmp_Input);
			std::cin >> tmp_Input;

			if (tmp_Input == "/save/")
			{
				NT4_Core.save(Save_FName);
				continue;
			}
			if (tmp_Input == "/load/")
			{
				NT4_Core.load(Save_FName);
				continue;
			}
			if (tmp_Input == "/train/")
			{
				train();
				continue;
			}
			if (tmp_Input == "/config/")
			{
				continue;
			}
			if (tmp_Input == "/set_BS/")
			{
				int tmp_Int = 0;
				std::cout << "\n Current block_Size: " << block_Size;
				std::cin >> block_Size;
			}

			if (tmp_Input == "/eval/")
			{
				eval();
				continue;
			}
		}
		return 1;
	}
};



























































//This relies on the NT4 script: './Scripts/GPT_Core.txt'.
//autoexec must have 'GPT_Core.txt' to work on startup. .
class c_GPT_1
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

	c_GPT_1(const int p_Block_Size = 25, const int p_Context_Size = 500)
	{
		block_Size = p_Block_Size;
		context_Size = p_Context_Size;

		Save_FName = "./cores/GPT.ssv";

		Input_Tokens = new uint64_t[block_Size];

		for (int cou_Index = 0; cou_Index < block_Size; cou_Index++)
		{
			Input_Tokens[cou_Index] = 0;
		}

		if (NT4_Core.get_Construct_Count() == 0)
		{
			std::cout << "\n\n";
			NT4_Core.register_Construct("Many_To_One", "Chrono");
			NT4_Core.register_Construct("Many_To_One", "Raw");
			NT4_Core.create_Construct_Connection("Raw", "Chrono");

			//This is a bit hacky, but atm we set the charing tier of the raw way above where it is reasonable.
			//This causes it to not actually charge, but it does fill the scaffold NULLCAN style with no reinforcement.
			NT4_Core.set_Charging_Tier("Raw", 100);
			NT4_Core.set_Action_Potential_Threshold("Chrono", 0.5);
		}

	}

	int get_Block_Size() { return block_Size; }
	int get_Context_Size() { return context_Size; }
	void set_Block_Size(const int p_Block_Size) { block_Size = p_Block_Size; }
	void set_Context_Size(const int p_Context_Size) { context_Size; }

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
		//NT4_Core.output_Input("Chrono");

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

		uint64_t Context_Token[5] = { 0, 0, 0, 0, 0 };

		NT4_Core.set_Input("Raw", "ghoul");
		NT4_Core.query("Raw");

		Context_Token[0] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "madness");
		NT4_Core.query("Raw");

		Context_Token[1] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "chased");
		NT4_Core.query("Raw");

		Context_Token[2] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "pursued");
		NT4_Core.query("Raw");

		Context_Token[3] = NT4_Core.get_Treetop_NID("Raw");

		NT4_Core.set_Input("Raw", "run");
		NT4_Core.query("Raw");

		Context_Token[4] = NT4_Core.get_Treetop_NID("Raw");



		while (tmp_Input != "/exit/")
		{
			NT4_Core.update_Config("Chrono");
			NT4_Core.update_Config("Raw");

			tmp_Count = 0;
			std::cout << "\n\n\n Filling Chrono...";
			std::cout << "\n" << (block_Size - 1) << " Tokens Prompt: [";
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
			std::cout << "\n\n\nPrompt:\n\n" << tmp_Seed_Text << "\n\n";
			std::cout << "\nResponse:\n\n";
			for (int cou_Index = 0; cou_Index < 100; cou_Index++)
			{
				if (!(cou_Index % 10)) { std::cout << "\n"; }
				//std::cout << "\n [" << cou_Index << " / " << 50;

				NT4_Core.reset_Output("Chrono");
				NT4_Core.reset_Output("Raw");
				
				NT4_Core.set_Input_uint("Chrono", block_Size, Input_Tokens);
				//NT4_Core.output_Input("Chrono");

				NT4_Core.query("Chrono");
				//NT4_Core.query_Spacial("Chrono");
				//NT4_Core.query("Chrono");

				NT4_Core.set_Input_uint("Chrono", 5, Context_Token);
				NT4_Core.query("Chrono");
				



				NT4_Core.set_Input_uint("Chrono", int(block_Size - 1), &(Input_Tokens[int(block_Size - 1)]));
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

				NT4::c_Lookup_Tree tmp_Output_Spread;
				NT4::c_Lookup_Tree tmp_Output_Spread_RC;
				NT4::c_Lookup_Tree tmp_Output_Spread_Occ;

				tmp_Output_Spread.reset();
				tmp_Output_Spread_RC.reset();
				tmp_Output_Spread_Occ.reset();

				//Create the output mapping by iterating through and collecting each NID + charge.
				double tmp_Charge = 0.0;
				double tmp_RC = 0.0;
				NT4::u_Tmp tmp_Union;
				tmp_Union.D = 0.0;
				NT4::u_Tmp tmp_Total_Charge;
				NT4::u_Tmp tmp_Total_RC;
				tmp_Total_Charge.D = 0.0;
				for (int cou_Iter = 0; cou_Iter < tmp_Top; cou_Iter++)
				{
					tmp_Input = NT4_Core.get_Output_Pattern("Raw", cou_Iter);
					tmp_RC = NT4_Core.get_Output_RC("Raw", cou_Iter);
					tmp_Charge = NT4_Core.get_Output_Charge("Raw", cou_Iter);

					//std::cout << "\n[" << cou_Iter << "] Pattern: <" << tmp_Input << "> {" << tmp_Charge << "} ";

					tmp_Output_Spread.search(tmp_Input);
					tmp_Output_Spread_RC.search(tmp_Input);
					tmp_Output_Spread_Occ.search(tmp_Input);

					if (tmp_Output_Spread.flg_Foundit)
					{ 
						tmp_Total_Charge.U = tmp_Output_Spread.get_Current_Data();
						tmp_Total_Charge.D += tmp_Charge;
						tmp_Output_Spread.set_Current_Data(tmp_Total_Charge.U);
						tmp_Total_RC.U = tmp_Output_Spread_RC.get_Current_Data();
						tmp_Total_RC.D += tmp_RC;
						tmp_Output_Spread_RC.set_Current_Data(tmp_Total_RC.U);
						tmp_Output_Spread_Occ.set_Current_Data(tmp_Output_Spread_Occ.get_Current_Data() + 1);

						//std::cout << "  +++  " << tmp_Total_Charge.D;
					}
					else
					{
						tmp_Total_Charge.D = tmp_Charge;
						tmp_Output_Spread.set_Current_Data(tmp_Total_Charge.U);
						tmp_Total_RC.D = tmp_RC;
						tmp_Output_Spread_RC.set_Current_Data(tmp_Total_RC.U);
						tmp_Output_Spread_Occ.set_Current_Data(0);

						//std::cout << "  ___  " << tmp_Total_Charge.D;
					}
				}
				//tmp_Output_Spread.output_Tree();

				NT4::c_Lookup_Node* tmp_LL = tmp_Output_Spread.Root;

				//----std::cout << "\n";

				const int MAX = 1000;

				std::string tmp_Top_Ten[MAX];
				double tmp_Top_Ten_Charge[MAX];

				//----std::cout << "\n\n\n--- 0";
				for (int cou_TT = 0; cou_TT < MAX; cou_TT++)
				{
					tmp_Top_Ten[cou_TT] = "";
					tmp_Top_Ten_Charge[cou_TT] = 0.0;
				}

				double tmp_Charge_PH = 0.0;
				std::string tmp_Input_PH = "";

				//----std::cout << "\n\n\n--- 1";
				while (tmp_LL != NULL)
				{
					tmp_Union.D = 0;
					tmp_Union.U = tmp_LL->Data;

					tmp_Output_Spread_Occ.search(tmp_LL->Name);
					tmp_Union.U = tmp_Output_Spread_Occ.get_Current_Data();
					double tmp_Occ = tmp_Union.D;
					if (tmp_Occ == 0.0) { tmp_Occ = 1; }

					tmp_Union.U = 0;

					tmp_Output_Spread_RC.search(tmp_LL->Name);
					NT4::u_Tmp tmp_RC_Union;
					tmp_RC_Union.U = tmp_Output_Spread_RC.get_Current_Data();
					if (tmp_RC_Union.D == 0)
					{
						tmp_RC_Union.D = 1;
					}

					//tmp_Union.D /= tmp_RC_Union.D;
					tmp_Union.D = (tmp_Union.D / tmp_RC_Union.D) * tmp_Occ;

					//----std::cout << "\n" << tmp_LL->Name << " {" << tmp_Union.D << "}";


					//----std::cout << "\n\n\n--- 2";
					if (tmp_Union.D > tmp_Top_Ten_Charge[MAX - 1])
					{
						tmp_Top_Ten[MAX - 1] = tmp_LL->Name;
						tmp_Top_Ten_Charge[MAX - 1] = tmp_Union.D;
					}

					//----std::cout << "\n\n\n--- 3";
					for (int cou_TT = (MAX - 1); cou_TT > 0; cou_TT--)
					{
						if (tmp_Top_Ten_Charge[cou_TT] > tmp_Top_Ten_Charge[cou_TT - 1])
						{
							tmp_Input_PH = "";
							tmp_Charge_PH = 0.0;

							tmp_Charge_PH = tmp_Top_Ten_Charge[cou_TT - 1];
							tmp_Input_PH = tmp_Top_Ten[cou_TT - 1];

							tmp_Top_Ten_Charge[cou_TT - 1] = tmp_Top_Ten_Charge[cou_TT];
							tmp_Top_Ten[cou_TT - 1] = tmp_Top_Ten[cou_TT];

							tmp_Top_Ten_Charge[cou_TT] = tmp_Charge_PH;
							tmp_Top_Ten[cou_TT] = tmp_Input_PH;
						}
					}

					tmp_LL = tmp_LL->Next;
				}

				/*
				std::cout << "\n Top Ten:";
				for (int cou_TT = 0; cou_TT < MAX; cou_TT++)
				{
					tmp_Output_Spread_Occ.search(tmp_Top_Ten[cou_TT]);
					std::cout << "\n[" << cou_TT << "] <" << tmp_Top_Ten[cou_TT] << ">  {" << tmp_Top_Ten_Charge[cou_TT] << "}" << " Occurance: " << tmp_Output_Spread_Occ.get_Current_Data();
				}
				*/
				//Pick one at random.
				tmp_Input = "";
				while (tmp_Input == "")
				{
					int tmp_Ran = (rand() % MAX);

					tmp_Input = tmp_Top_Ten[tmp_Ran];
				}

				/*
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
				*/
				std::cout << tmp_Input << " ";

				tmp_Output_Text += tmp_Input;
				tmp_Output_Text += " ";

				//std::cout << "\n ";
				NT4_Core.set_Input("Raw", tmp_Input);
				NT4_Core.query("Raw");

				shift_Input();

				Input_Tokens[block_Size - 1] = NT4_Core.get_Treetop_NID("Raw");
			}
			std::cout << "\n\n";
			//std::cout << "\n\n\nPrompt: [" << tmp_Seed_Text << "]";
			//std::cout << "Response: [" << tmp_Output_Text << "]";
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
			std::cout << "\n /set_BS/ - Set the block_Size and the context_Size. This is separate from the ML hyperparams.";
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
			if (tmp_Input == "/set_BS/")
			{
				int tmp_Int = 0;
				std::cout << "\n Current block_Size: " << get_Block_Size();
				std::cin >> tmp_Int;
				set_Block_Size(tmp_Int);

				tmp_Int = 0;
				std::cout << "\n Current context_Size: " << get_Context_Size();
				std::cin >> tmp_Int;
				set_Block_Size(tmp_Int);
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