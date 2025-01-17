

class c_Agent
{
public:

	int HP;

	bool flg_Alive;
	bool flg_Spawn;

	float Goal;
	std::string Color;

	float X;
	float Y;
	float Speed;

	std::string Name;

	c_Agent()
	{
		HP = 1;

		flg_Alive = true;
		flg_Spawn = false;

		X = 0;
		Y = 0;

		Speed = 0.01 + ((rand() % 5) * 0.01);

		Goal = 80;
		Color = "Gray";
		Name = "Agent";
	}

	void output()
	{
		std::cout << "\n " << Name;

		std::cout << " X " << X << " Y " << Y << " HP " << HP;
	}

	void update(float p_Temps[5], int p_Max_X, int p_Max_Y)
	{
		float tmp_Best = 99 * 99;
		int tmp_Best_Index = -99;

		float tmp_Current = -99;

		float tmp_MSE[5];

		for (int cou_M = 0; cou_M < 5; cou_M++)
		{
			tmp_MSE[cou_M] = (Goal - p_Temps[cou_M]) * (Goal - p_Temps[cou_M]);
		}

		if (tmp_MSE[0] > tmp_MSE[2]) { X += Speed; }
		if (tmp_MSE[0] < tmp_MSE[2]) { X -= Speed; }

		if (tmp_MSE[1] > tmp_MSE[3]) { Y -= Speed; }
		if (tmp_MSE[1] < tmp_MSE[3]) { Y += Speed; }

		if (rand() % 2)
		{
			if (rand() % 2)
			{
				if (tmp_MSE[0] > tmp_MSE[2]) { X += Speed; }
				if (tmp_MSE[0] < tmp_MSE[2]) { X -= Speed; }
			}
			else
			{
				if (tmp_MSE[1] > tmp_MSE[3]) { Y -= Speed; }
				if (tmp_MSE[1] < tmp_MSE[3]) { Y += Speed; }
			}
		}

		if (X <= 0) { X = 1; }
		if (Y <= 0) { Y = 1; }
		if (X >= p_Max_X) { X = (p_Max_X - 2); }
		if (Y >= p_Max_Y) { Y = (p_Max_Y - 2); }

		tmp_Current = (Goal - p_Temps[4]) * (Goal - p_Temps[4]);

		//std::cout << "\n " << tmp_Current << " = (" << Goal << " - " << p_Temps[4] << ")* (" << Goal << " - " << p_Temps[4] << ")";

		if (tmp_Current > 2000) { HP--; }
		if (tmp_Current < 1000) { HP += 1; }
		if (tmp_Current < 10) { HP += 5; }

		if (HP > 10) { HP = 10; }
		if (HP <= 0) { flg_Alive = false; HP = 0; }
		if (HP >= 9) { flg_Alive = true; }
	}
};

class c_Sim
{
public:

	int Iteration;

	int O2;
	int O2_Delta;

	int Temp;
	int Temp_Delta;

	int O2_Pump;
	int Heater;

	c_Sim()
	{
		Iteration = 0;

		O2 = 5;
		O2_Delta = 1;

		Temp = 5;
		Temp_Delta = 1;

		O2_Pump = 0;
		Heater = 0;
	}

	// Afferent Sensors - Goals
	// O2      | -1/iteration //Breathing   | Goal: 5 | G:5 Y:(4 & 6)      R: (< 4 & > 6)  | Priority: 2  
	// Temp    | -2/iteration //Winter      | Goal: 8 | G:8 Y:(6-7 & 9-10) R: (< 6 & > 10) | Priority: 2  

	// Actuator Systems - Feedback
	// O2 Pump | +4/Iteration [O2]          | -1/Iteration [Temperature] | if (O2 < 5) (Heater = 1) else (Heater = 0)
	// Heater  | +3/Iteration [Temperature] |                            | if (Temp < 8) (Heater = 1) else (Heater = 0)

	int iterate(int p_O2_Pump = -1, int p_Heater = -1)
	{
		Iteration++;

		//Environmental
		int tmp_O2 = O2;
		O2 -= 1; //Breathing

		int tmp_Temp = Temp;
		Temp -= 2; //Winter

		//Calculate actuator effects.
		if (O2_Pump) { O2 += 4; Temp -= 1; } //O2 Pump
		if (Heater) { Temp += 3; } //Heater

		//Calculate the environmental changes based on the previous iterations.
		if (O2 < tmp_O2) { O2_Delta = 0; }
		if (O2 == tmp_O2) { O2_Delta = 1; }
		if (O2 > tmp_O2) { O2_Delta = 2; }

		if (Temp < tmp_Temp) { Temp_Delta = 0; }
		if (Temp == tmp_Temp) { Temp_Delta = 1; }
		if (Temp > tmp_Temp) { Temp_Delta = 2; }

		if ((p_Heater != -1) && (p_O2_Pump != -1))
		{
			Heater = p_Heater;
			O2_Pump = p_O2_Pump;
		}
		else
		{
			//Calculate the actuator state.
			if (tmp_Temp < 8) { Heater = 1; }
			if (tmp_O2 < 5) { O2_Pump = 1; }

			if (tmp_Temp >= 8) { Heater = 0; }
			if (tmp_O2 >= 5) { O2_Pump = 0; }
		}

		return Iteration;
	}

	void output()
	{
		std::cout << "\n";
		std::cout << Iteration;
		std::cout << "\nencode 0 set_input 0 O2 " << O2;
		std::cout << "\nencode 0 set_input 0 O2_Delta " << O2_Delta;

		std::cout << "\nencode 0 set_input 0 Temp " << Temp;
		std::cout << "\nencode 0 set_input 0 Temp_Delta " << Temp_Delta;

		std::cout << "\nencode 0 set_input 0 O2_Pump " << O2_Pump;
		std::cout << "\nencode 0 set_input 0 Heater " << Heater;
	}

	void output_F(std::string p_FName)
	{
		std::ofstream tmp_F;

		tmp_F.open(p_FName, std::ios::app);


		tmp_F << "\n";
		tmp_F << "\noutput_newline";
		tmp_F << "\nset_input 0 ar_O2 " << O2 << " /end/ encode 0 gather_treetop_node 0 ";
		tmp_F << "\nset_input 1 ac_O2_Delta " << O2_Delta << " /end/ encode 1 gather_treetop_node 1 ";
		tmp_F << "\nset_input 2 em_O2_Pump " << O2_Pump << " /end/ encode 2 gather_treetop_node 2 ";

		tmp_F << "\nset_input 3 ar_Temp " << Temp << " /end/ encode 3 gather_treetop_node 3 ";
		tmp_F << "\nset_input 4 ac_Temp_Delta " << Temp_Delta << " /end/ encode 4 gather_treetop_node 4 ";
		tmp_F << "\nset_input 5 em_Heater " << Heater << " /end/ encode 5 gather_treetop_node 5 ";

		tmp_F.close();
	}
};


class c_Cell
{
public:

	float Temp;

	std::vector<float> Factors;

	int Direction;
	float Magnitude_X;
	float Magnitude_Y;

	int Type;

	c_Cell(int p_Factor_Count = 1)
	{
		Temp = 0;
		Type = 1;
		Direction = 0;

		Magnitude_X = 0;
		Magnitude_Y = 0;

		Factors.resize(p_Factor_Count);
	}


};

class c_Sensor
{
public:

	int X;
	int Y;
	float Previous;
	float Delta;
	float Current;

	c_Sensor()
	{
		X = 0;
		Y = 0;
		Previous = 0;
		Current = 0;
		Delta = 0;
	}

	float get_Delta()
	{
		return Delta;
	}

	void set_Current(float p_Current)
	{
		Previous = Current;
		Current = p_Current;

		Delta = Current - Previous;
	}
};

class c_Actuator
{
public:

	int X;
	int Y;

	bool flg_On_Off;

	float On_Temp;

	float Current;

	c_Actuator()
	{
		X = 0;
		Y = 0;
		Current = 0;
		On_Temp = 100;

		flg_On_Off = false;
	}

	int is_On(){ if (flg_On_Off) { return 1; } else { return -1; } }

	bool turn_Off() { flg_On_Off = false; return flg_On_Off; }
	bool turn_On() { flg_On_Off = true; return flg_On_Off; }

	float get_Temp() { return On_Temp; }
};

class c_Historical_DB
{
	std::vector<std::vector<std::vector<u_Data>>> Hist;
	int Chrono_Depth;
	int Raw_Depth;

public:

	c_Historical_DB()
	{
		Chrono_Depth = 0;
		Raw_Depth = 0;
	}

	void reset()
	{
		Hist.clear();
	}

	void set_Chrono_Depth(int p_Chrono_Depth)
	{
		Chrono_Depth = p_Chrono_Depth;
	}

	void set_Raw_Depth(int p_Raw_Depth)
	{
		Raw_Depth = p_Raw_Depth;
	}

	int get_Chrono_Depth()
	{
		return Chrono_Depth;
	}

	int get_Raw_Depth()
	{
		return Raw_Depth;
	}

	std::vector<u_Data> get_Entry(int p_Step, int p_Raw)
	{
		//Build the entry.
		std::vector<u_Data> return_Data;

		for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
		{
			//---std::cout << "\n Chrono[" << cou_Chrono << "]";
			//---std::cout << " " << Hist[cou_Step][cou_Chrono][cou_Raw].D;
			return_Data.push_back(Hist[p_Step][cou_Chrono][p_Raw]);
		}

		return return_Data;
	}

	void add_To_Hist(std::vector<std::vector<u_Data>> p_Data)
	{
		//So we need history for every frame
		//One datatype per history DB
		//We store the projection associated with each index
		//As we step away from the peojections we do the comparison the ground truth to get the scores

		//One frame per tick so far, this is pushed on.
		//One array per frame to hold the prediction or current stats.

		//This tick.
		//Hist.push_back(std::vector<std::vector<u_Data>>());
		Hist.push_back(p_Data);
		return;

		//Hold the current frame chrono block depth.
		Hist[Hist.size() - 1].resize(p_Data.size());

		//Copy the frame data over.
		for (int cou_Chrono = 0; cou_Chrono < p_Data.size(); cou_Chrono++)
		{
			Hist[Hist.size() - 1][cou_Chrono].resize(p_Data[cou_Chrono].size());
			for (int cou_Raw = 0; cou_Raw < p_Data[cou_Chrono].size(); cou_Raw++)
			{
				Hist[Hist.size() - 1][cou_Chrono][cou_Raw] = p_Data[cou_Chrono][cou_Raw];
			}
		}
	}

	int get_Entry_Count()
	{
		return Hist.size();
	}

	void output_DB()
	{
		std::cout << "\n\n output_DB()";
		for (int cou_Step = 0; cou_Step < Hist.size(); cou_Step++)
		{
			std::cout << "\n Index[" << cou_Step << "]";
			for (int cou_Raw = 0; cou_Raw < Raw_Depth; cou_Raw++)
			{
				std::cout << "\n ... Raw[" << cou_Raw << "]";
				for (int cou_Chrono = 0; cou_Chrono < Chrono_Depth; cou_Chrono++)
				{
					//---std::cout << "\n Chrono[" << cou_Chrono << "]";
					std::cout << " " << Hist[cou_Step][cou_Chrono][cou_Raw].D;
				}
			}
		}
	}
};

class c_Map_Sim
{
	std::vector<std::vector<std::vector<c_Cell>>> Map;
	bool Current_Frame;
	bool Next_Frame;

	std::vector<c_Sensor> Sensors;

	std::vector<c_Actuator> Actuators;

	int Width;

	int Height;

	std::string Filename;

	int Tick;

	std::vector<c_Agent> The_Outsiders;

public:

	c_Map_Sim(int p_Width, int p_Height)
	{
		Current_Frame = 0;
		Next_Frame = 1;

		Filename = "./Scripts/";

		new_Map(p_Width, p_Height);

		Tick = 0;
	}

	void init_Agents(int p_Count)
	{
		The_Outsiders.resize(p_Count * 3);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			The_Outsiders[cou_Index].X = (rand() % Width);
			The_Outsiders[cou_Index].Y = (rand() % Height);
			The_Outsiders[cou_Index].Name = "Agent_" + std::to_string(cou_Index);
			The_Outsiders[cou_Index].Goal = 70;
			The_Outsiders[cou_Index].Color = "blue";
		}

		for (int cou_Index = p_Count; cou_Index < (p_Count * 2); cou_Index++)
		{
			The_Outsiders[cou_Index].X = (rand() % Width);
			The_Outsiders[cou_Index].Y = (rand() % Height);
			The_Outsiders[cou_Index].Name = "Agent_" + std::to_string(cou_Index);
			The_Outsiders[cou_Index].Goal = 70;
			The_Outsiders[cou_Index].Color = "green";
		}

		for (int cou_Index = (p_Count * 2); cou_Index < (p_Count * 3); cou_Index++)
		{
			The_Outsiders[cou_Index].X = (rand() % Width);
			The_Outsiders[cou_Index].Y = (rand() % Height);
			The_Outsiders[cou_Index].Name = "Agent_" + std::to_string(cou_Index);
			The_Outsiders[cou_Index].Goal = 50;
			The_Outsiders[cou_Index].Color = "red";
		}
	}

	//B 70
	//G 70
	//R 50

	int get_Agent_Count()
	{
		return The_Outsiders.size();
	}

	int get_Width() { return Width; }
	int get_Height() { return Height; }

	void swap_Frame()
	{
		Current_Frame = !Current_Frame;
		Next_Frame = !Current_Frame;
		//std::cout << "\n[" << Current_Frame << " " << Next_Frame << "]";
	}

	void set_Agent_XY(int p_Agent, float p_X, float p_Y)
	{
		The_Outsiders[p_Agent].X = p_X;
		The_Outsiders[p_Agent].Y = p_Y;
	}

	void set_Agent_HP(int p_Agent, int p_HP)
	{
		The_Outsiders[p_Agent].HP = p_HP;
	}

	int add_Sensor(int p_X, int p_Y)
	{
		Sensors.push_back(c_Sensor());

		Sensors[Sensors.size() - 1].X = p_X;
		Sensors[Sensors.size() - 1].Y = p_Y;

		Map[Current_Frame][p_X][p_Y].Type = 2;
		Map[Next_Frame][p_X][p_Y].Type = 2;

		return int(Sensors.size()) - 1;
	}

	int add_Actuator(int p_X, int p_Y, float p_Temp)
	{
		Actuators.push_back(c_Actuator());

		Actuators[Actuators.size() - 1].X = p_X;
		Actuators[Actuators.size() - 1].Y = p_Y;
		Actuators[Actuators.size() - 1].On_Temp = p_Temp;

		Map[Current_Frame][p_X][p_Y].Type = 3;
		Map[Next_Frame][p_X][p_Y].Type = 3;

		return int(Actuators.size()) - 1;
	}

	int get_Actuator_State(int p_Actuator)
	{
		return Actuators[p_Actuator].is_On();
	}

	float get_Actuator_Temp(int p_Actuator)
	{
		return Actuators[p_Actuator].get_Temp();
	}

	void turn_Actuator_On(int p_Actuator)
	{
		Actuators[p_Actuator].turn_On();
	}

	void turn_Actuator_Off(int p_Actuator)
	{
		Actuators[p_Actuator].turn_Off();
	}
	
	float get_Sensor_Data(int p_Sensor)
	{
		Sensors[p_Sensor].set_Current(Map[Current_Frame][Sensors[p_Sensor].X][Sensors[p_Sensor].Y].Temp);
		return Sensors[p_Sensor].Current;
	}
	
	float get_Sensor_Delta(int p_Sensor)
	{
		return Sensors[p_Sensor].get_Delta();
	}

	void new_Map(int p_Width, int p_Height)
	{
		Width = p_Width;
		Height = p_Height;

		std::cout << "\n Creating Map(" << Width << ", " << Height << ")";

		Map.resize(2);

		Map[0].resize(p_Width);
		Map[1].resize(p_Width);
		for (int cou_X = 0; cou_X < p_Width; cou_X++)
		{
			Map[0][cou_X].resize(p_Height);
			Map[1][cou_X].resize(p_Height);
		}

		std::cout << "\n Finished Map";
	}

	void reset_Tick()
	{
		Tick = 0;
	}

	int get_Tick()
	{
		return Tick;
	}

	void set_Temp(int p_X, int p_Y, float p_Temp)
	{
		Map[Current_Frame][p_X][p_Y].Temp = p_Temp;
	}

	void set_Map_Temp(float p_Temp)
	{
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				set_Temp(cou_X, cou_Y, p_Temp);
			}
		}
	}

	void set_Type(int p_X, int p_Y, int p_Type)
	{
		Map[Current_Frame][p_X][p_Y].Type = p_Type;
		Map[Next_Frame][p_X][p_Y].Type = p_Type;
	}

	bool check_XY(int p_X, int p_Y)
	{
		if (p_X < 0) { return 0; }
		if (p_Y < 0) { return 0; }
		if (p_X >= Width) { return 0; }
		if (p_Y >= Height) { return 0; }

		if (Map[Current_Frame][p_X][p_Y].Type == 0) { return 0; }

		return 1;
	}

	void diffuse_Temp(int p_X, int p_Y)
	{
		float tmp_Total = 0.0;
		float tmp_Count = 0.0;
		float tmp_Current = 0.0;
		float tmp_Flow = 0.0;

		if (!check_XY(p_X, p_Y)) { return; }
		
		//Find the difference for each side. Average the lowest only.
		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		float tmp_Current_Diff = 0;
		
		Map[Next_Frame][p_X][p_Y].Direction = -1;
		Map[Next_Frame][p_X][p_Y].Magnitude_X = 0;
		Map[Next_Frame][p_X][p_Y].Magnitude_Y = 0;
		if (check_XY(p_X + 1, p_Y)) { if ((tmp_Current - Map[Current_Frame][p_X + 1][p_Y].Temp) > tmp_Current_Diff) { tmp_Current_Diff = (tmp_Current - Map[Current_Frame][p_X + 1][p_Y].Temp); Map[Next_Frame][p_X][p_Y].Direction = 0; } }
		if (check_XY(p_X, p_Y + 1)) { if ((tmp_Current - Map[Current_Frame][p_X][p_Y + 1].Temp) > tmp_Current_Diff) { tmp_Current_Diff = (tmp_Current - Map[Current_Frame][p_X][p_Y + 1].Temp); Map[Next_Frame][p_X][p_Y].Direction = 1; } }
		if (check_XY(p_X - 1, p_Y)) { if ((tmp_Current - Map[Current_Frame][p_X - 1][p_Y].Temp) > tmp_Current_Diff) { tmp_Current_Diff = (tmp_Current - Map[Current_Frame][p_X - 1][p_Y].Temp); Map[Next_Frame][p_X][p_Y].Direction = 2; } }
		if (check_XY(p_X, p_Y - 1)) { if ((tmp_Current - Map[Current_Frame][p_X][p_Y - 1].Temp) > tmp_Current_Diff) { tmp_Current_Diff = (tmp_Current - Map[Current_Frame][p_X][p_Y - 1].Temp); Map[Next_Frame][p_X][p_Y].Direction = 3; } }

		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;

		int tmp_XX = 0;
		int tmp_YY = 0;

		Map[Next_Frame][p_X][p_Y].Magnitude_X = 0;
		Map[Next_Frame][p_X][p_Y].Magnitude_Y = 0;


		if (check_XY(p_X + 1, p_Y)) 
		{
			Map[Next_Frame][p_X][p_Y].Magnitude_X = (Map[Current_Frame][p_X + 1][p_Y].Temp - tmp_Current);
		}

		if (check_XY(p_X - 1, p_Y)) 
		{
			Map[Next_Frame][p_X][p_Y].Magnitude_X -= (Map[Current_Frame][p_X - 1][p_Y].Temp - tmp_Current);
		}
		
		if (check_XY(p_X, p_Y + 1)) 
		{
			Map[Next_Frame][p_X][p_Y].Magnitude_Y = (Map[Current_Frame][p_X][p_Y + 1].Temp - tmp_Current);
		}

		if (check_XY(p_X, p_Y - 1)) 
		{
			Map[Next_Frame][p_X][p_Y].Magnitude_Y -= (Map[Current_Frame][p_X][p_Y - 1].Temp - tmp_Current);
		}


		/*
		if ((tmp_Direction == 0) && (check_XY(p_X + 1, p_Y)))
		{
			tmp_Flow = (tmp_Current + Map[Current_Frame][p_X + 1][p_Y].Temp) / 2;
		}
		
		if ((tmp_Direction == 1) && (check_XY(p_X, p_Y + 1)))
		{
			tmp_Flow = (tmp_Current + Map[Current_Frame][p_X][p_Y + 1].Temp) / 2;
		}
		
		if ((tmp_Direction == 2) && (check_XY(p_X - 1, p_Y)))
		{
			tmp_Flow = (tmp_Current + Map[Current_Frame][p_X - 1][p_Y].Temp) / 2;
		}
		
		if ((tmp_Direction == 3) && (check_XY(p_X, p_Y - 1)))
		{
			tmp_Flow = (tmp_Current + Map[Current_Frame][p_X][p_Y - 1].Temp) / 2;
		}

		*/
		
		//std::cout << "\n C: " << Map[Current_Frame][p_X][p_Y].Temp;
		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		if (check_XY(p_X + 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X + 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X, p_Y + 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y + 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X - 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X - 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X, p_Y - 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y - 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }

		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		if (check_XY(p_X, p_Y - 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y - 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X + 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X + 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X, p_Y + 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y + 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X - 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X - 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }

		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		if (check_XY(p_X - 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X - 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X, p_Y - 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y - 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X + 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X + 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X, p_Y + 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y + 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }

		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		if (check_XY(p_X, p_Y + 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y + 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X - 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X - 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X, p_Y - 1)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X][p_Y - 1].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }
		if (check_XY(p_X + 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X + 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }

		//tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		//if (check_XY(p_X + 1, p_Y)) { tmp_Current = (tmp_Current + Map[Current_Frame][p_X + 1][p_Y].Temp) / 2; tmp_Count++; tmp_Total += tmp_Current; }

		Map[Next_Frame][p_X][p_Y].Temp = (tmp_Total /= tmp_Count);
		
		//std::cout << "\n C: " << Map[Current_Frame][p_X][p_Y].Temp;
		//std::cout << "\n N: " << Map[Next_Frame][p_X][p_Y].Temp;
	}

	float get_Temp(int p_X, int p_Y)
	{
		return Map[Current_Frame][p_X][p_Y].Temp;
	}

	void chill_Map(float p_Chill)
	{
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				Map[Current_Frame][cou_X][cou_Y].Temp -= p_Chill;
				if (Map[Current_Frame][cou_X][cou_Y].Temp < 0){ Map[Current_Frame][cou_X][cou_Y].Temp = 0; }
			}
		}
	}

	void update()
	{
		for (int cou_Index = 0; cou_Index < Actuators.size(); cou_Index++)
		{
			if (Actuators[cou_Index].is_On() == 1)
			{
				Map[Current_Frame][Actuators[cou_Index].X][Actuators[cou_Index].Y].Temp = Actuators[cou_Index].get_Temp();
			}
		}

		for (int cou_X = 0; cou_X < (Width); cou_X++)
		{
			for (int cou_Y = 0; cou_Y < (Height); cou_Y++)
			{
				diffuse_Temp(cou_X, cou_Y);
			}
		}

		float tmp_Temps[5] = { -99, -99, -99, -99, -99 };

		//std::cout << "\n";
		for (int cou_Index = 0; cou_Index < The_Outsiders.size(); cou_Index++)
		{

			tmp_Temps[0] = -99;
			tmp_Temps[1] = -99;
			tmp_Temps[2] = -99;
			tmp_Temps[3] = -99;
			tmp_Temps[4] = -99;

			if (check_XY(The_Outsiders[cou_Index].X - 1, The_Outsiders[cou_Index].Y)) { tmp_Temps[0] = Map[Current_Frame][The_Outsiders[cou_Index].X - 1][The_Outsiders[cou_Index].Y].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y + 1)) { tmp_Temps[1] = Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y + 1].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X + 1, The_Outsiders[cou_Index].Y)) { tmp_Temps[2] = Map[Current_Frame][The_Outsiders[cou_Index].X + 1][The_Outsiders[cou_Index].Y].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y - 1)) { tmp_Temps[3] = Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y - 1].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y)) { tmp_Temps[4] = Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp; }
			//std::cout << "\n MT " << Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp;
			//std::cout << "\n TM " << tmp_Temps[4];
			The_Outsiders[cou_Index].update(tmp_Temps, Width, Height);

			The_Outsiders[cou_Index].X -= (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_X) * (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_X);
			
			if (The_Outsiders[cou_Index].X >= Width) { The_Outsiders[cou_Index].X = Width - 1; }
			if (The_Outsiders[cou_Index].X <= 0) { The_Outsiders[cou_Index].X = 1; }

			The_Outsiders[cou_Index].Y -= (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_Y) * (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_Y);

			if (The_Outsiders[cou_Index].Y >= Height) { The_Outsiders[cou_Index].Y = Height - 1; }
			if (The_Outsiders[cou_Index].Y <= 0) { The_Outsiders[cou_Index].Y = 1; }

			if (The_Outsiders[cou_Index].flg_Alive == false) { continue; }
			/*
			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 0)
			{
				The_Outsiders[cou_Index].Y += The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].Y >= Height) { The_Outsiders[cou_Index].Y = Height - 1; }
			}

			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 1)
			{
				The_Outsiders[cou_Index].X += The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].X >= Width) { The_Outsiders[cou_Index].X = Width - 1; }
			}

			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 2)
			{
				The_Outsiders[cou_Index].Y -= The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].Y <= 0) { The_Outsiders[cou_Index].Y = 1; }
			}

			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 3)
			{
				The_Outsiders[cou_Index].X -= The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].X <= 0) { The_Outsiders[cou_Index].X = 1; }
			}*/

			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y))
			{
				if (The_Outsiders[cou_Index].Color == "red")
				{
					Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp += 2;
				}
				if (The_Outsiders[cou_Index].Color == "blue")
				{
					Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp++;
				}
				if (The_Outsiders[cou_Index].Color == "green")
				{
					Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp--;
					Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp--;
					Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp--;
					Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp--;
				}
			}

		}


		swap_Frame();

		Tick++;
	}

	void output_Agents()
	{
		std::cout << "\n";
		for (int cou_Index = 0; cou_Index < The_Outsiders.size(); cou_Index++)
		{
			The_Outsiders[cou_Index].output();
		}
	}

	void view_Map()
	{
		std::string tmp_Out = "\n";
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			tmp_Out += "\n[";
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				//std::cout << "[" << Map[Current_Frame][cou_X][cou_Y].Temp << "]";

				if (Map[Current_Frame][cou_X][cou_Y].Type == 0)
				{
					tmp_Out += char(178);
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Type == 2)
				{
					tmp_Out += 'S';
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Type == 3)
				{
					tmp_Out += 'A';
					continue;
				}
				if (char(Map[Current_Frame][cou_X][cou_Y].Temp) > 20)
				{
					tmp_Out += char(Map[Current_Frame][cou_X][cou_Y].Temp);
				}
				else
				{
					tmp_Out += " ";
				}
			}
			tmp_Out += "]";
			if (cou_X < Sensors.size())
			{
				tmp_Out += " S[" + std::to_string(cou_X) + "]: [";
				tmp_Out = tmp_Out + "(" + std::to_string(Sensors[cou_X].X) + ", " + std::to_string(Sensors[cou_X].Y) + ") ";
				tmp_Out += "\t" + std::to_string(get_Sensor_Data(cou_X) + 0.000001);
				tmp_Out += "   \tDelta: " + std::to_string(get_Sensor_Delta(cou_X));
				tmp_Out += "] ";
			}
			if (cou_X > Sensors.size())
			{
				int tmp_Index = (cou_X - Sensors.size() - 1);

				if (tmp_Index < Actuators.size())
				{
					tmp_Out += " A[" + std::to_string(tmp_Index) + "]: [";
					if (get_Actuator_State(tmp_Index) == -1) { tmp_Out += " ----[OFF] "; }
					if (get_Actuator_State(tmp_Index) == 0) { tmp_Out += " [BROKEN] "; }
					if (get_Actuator_State(tmp_Index) == 1) { tmp_Out += " [ON]---- "; }
					//tmp_Out += " On/Off: " + std::to_string(get_Actuator_State(tmp_Index));
					tmp_Out += " Temp: " + std::to_string(get_Actuator_Temp(tmp_Index));
					tmp_Out = tmp_Out + "(" + std::to_string(Actuators[tmp_Index].X) + ", " + std::to_string(Actuators[tmp_Index].Y) + ") ";
					tmp_Out += "] ";
				}
			}
		}
		//tmp_Out += "\n";

		std::cout << tmp_Out;
	}

	
	void write_Map_Tiles(std::string p_FName)
	{
		std::ofstream SF;
		std::string tmp_FName = p_FName + "." + get_Padded(10, Tick) + ".MAP.ssv";
		SF.open(tmp_FName, std::ios::trunc);
		
		std::ofstream SFL;
		std::string tmp_LFName = p_FName + "." + get_Padded(10, Tick) + ".MAP.lbl";
		SFL.open(tmp_LFName, std::ios::trunc);

		std::vector<std::vector<float>> tmp_Map;
		tmp_Map.resize(Width);

		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			tmp_Map[cou_X].resize(Height);
		}

		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				tmp_Map[cou_X][cou_Y] = Map[Current_Frame][cou_X][cou_Y].Temp;
			}
		}
		
		for (int cou_S = 0; cou_S < Sensors.size(); cou_S++)
		{
			tmp_Map[Sensors[cou_S].X][Sensors[cou_S].Y] = 0.0;
			SFL << "S_" << std::to_string(cou_S) << " " << Sensors[cou_S].Y << " " << Sensors[cou_S].X << " 8 white [" << int(Sensors[cou_S].Current) << "]\n";
		}

		for (int cou_A = 0; cou_A < Actuators.size(); cou_A++)
		{
			tmp_Map[Actuators[cou_A].X][Actuators[cou_A].Y] = 500.0;
			if (Actuators[cou_A].flg_On_Off == 1)
			{
				SFL << "A_" << std::to_string(cou_A) << " " << Actuators[cou_A].Y << " " << Actuators[cou_A].X << " 8 red [" << Actuators[cou_A].get_Temp() << "]\n";
			}
			if (Actuators[cou_A].flg_On_Off == 0)
			{
				SFL << "A_" << std::to_string(cou_A) << " " << Actuators[cou_A].Y << " " << Actuators[cou_A].X << " 8 gray [" << Actuators[cou_A].get_Temp() << "]\n";
			}
		}

		for (int cou_Agent = 0; cou_Agent < The_Outsiders.size(); cou_Agent++)
		{
			if (The_Outsiders[cou_Agent].flg_Alive == 0) { continue; }

			SFL << "A" << cou_Agent << " " << The_Outsiders[cou_Agent].Y << " " << The_Outsiders[cou_Agent].X << " 4 " << The_Outsiders[cou_Agent].Color << " +\n";
		}

		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			if (cou_X > 0)
			{
				SF << "\n";
			}
			SF << Tick << " ";
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				SF << tmp_Map[cou_X][cou_Y] << " ";
			}
		}

		SF.close();
		SFL.close();
	}

	void write_Map_Temp(int p_Sensor, std::string p_FName)
	{
		std::ofstream SF;
		std::string tmp_FName = p_FName + "." + std::to_string(p_Sensor) + ".ssv";
		SF.open(tmp_FName, std::ios::app);

		SF << Tick << " " << get_Sensor_Data(p_Sensor);

		SF << "\n";
		SF.close();
	}

	void write_Map_Temps(std::string p_FName)
	{
		float tmp_Total = 0;

		for (int cou_S = 0; cou_S < Sensors.size(); cou_S++)
		{
			write_Map_Temp(cou_S, p_FName);
			tmp_Total += get_Sensor_Data(cou_S);
		}

		tmp_Total /= Sensors.size();

		std::ofstream SF;
		std::string tmp_FName = p_FName + ".Temp_Avg.ssv";
		SF.open(tmp_FName, std::ios::app);

		SF << Tick << " " << tmp_Total;

		SF << "\n";
		SF.close();
	}

	void write_Map_MSE(int p_Sensor, std::string p_FName)
	{
		std::ofstream SF;
		std::string tmp_FName = p_FName + "." + std::to_string(p_Sensor) + ".ssv";
		SF.open(tmp_FName, std::ios::app);

		SF << Tick << " " << ((80 - get_Sensor_Data(p_Sensor)) * (80 - get_Sensor_Data(p_Sensor)));

		SF << "\n";
		SF.close();
	}

	void write_Map_MSE(std::string p_FName)
	{
		float tmp_Total_MSE = 0;
		for (int cou_S = 0; cou_S < Sensors.size(); cou_S++)
		{
			write_Map_MSE(cou_S, p_FName);

			tmp_Total_MSE += ((80 - get_Sensor_Data(cou_S)) * (80 - get_Sensor_Data(cou_S)));
		}

		tmp_Total_MSE /= Sensors.size();

		std::ofstream SF;
		std::string tmp_FName = p_FName + ".MSE_Avg.ssv";
		SF.open(tmp_FName, std::ios::app);

		SF << Tick << " " << tmp_Total_MSE;

		SF << "\n";
		SF.close();
	}

	int get_Actuator_Count()
	{
		return Actuators.size();
	}

	int get_Sensor_Count()
	{
		return Sensors.size();
	}

	void draw_Building(int p_X0, int p_Y0, int p_X1, int p_Y1)
	{
		for (int cou_X = p_X0; cou_X <= p_X1; cou_X++)
		{
			set_Type(cou_X, p_Y0, 0);
			set_Type(cou_X, p_Y1, 0);
		}

		for (int cou_Y = p_Y0; cou_Y <= p_Y1; cou_Y++)
		{
			set_Type(p_X0, cou_Y, 0);
			set_Type(p_X1, cou_Y, 0);
		}

		//p_Map->set_Type(23, 20, 1);
	}
};

void draw_Building(c_Map_Sim * p_Map, int p_X0, int p_Y0, int p_X1, int p_Y1)
{
	for (int cou_X = p_X0; cou_X <= p_X1; cou_X++)
	{
		p_Map->set_Type(cou_X, p_Y0, 0);
		p_Map->set_Type(cou_X, p_Y1, 0);
	}

	for (int cou_Y = p_Y0; cou_Y <= p_Y1; cou_Y++)
	{
		p_Map->set_Type(p_X0, cou_Y, 0);
		p_Map->set_Type(p_X1, cou_Y, 0);
	}

	//p_Map->set_Type(23, 20, 1);
}

void runmap()
{
	c_Map_Sim Map(60, 175);

	int tmp_X = 0;
	int tmp_Y = 0;
	float tmp_Temp = 0.0;
	float tmp_Sensor = 0;
	bool flg_Ranran = false;
	bool flg_Heater = false;
	bool flg_AC = false;


	draw_Building(&Map, 10, 10, 40, 40);
	draw_Building(&Map, 10, 45, 40, 75);
	draw_Building(&Map, 10, 80, 40, 110);
	draw_Building(&Map, 10, 115, 40, 145);

	draw_Building(&Map, 9, 9, 46, 146);
	draw_Building(&Map, 40, 10, 45, 145);

	draw_Building(&Map, 46, 10, 56, 20);

	Map.set_Type(11, 40, 1);
	Map.set_Type(11, 75, 1);
	Map.set_Type(11, 110, 1);
	Map.set_Type(11, 145, 1);

	Map.set_Type(39, 40, 1);
	Map.set_Type(39, 75, 1);
	Map.set_Type(39, 110, 1);
	Map.set_Type(39, 145, 1);

	Map.set_Type(11, 45, 1);
	Map.set_Type(11, 80, 1);
	Map.set_Type(11, 115, 1);

	Map.set_Type(9, 25, 1);
	Map.set_Type(10, 25, 1);

	Map.set_Type(40, 41, 1);
	Map.set_Type(40, 42, 1);
	Map.set_Type(40, 43, 1);
	Map.set_Type(40, 44, 1);

	Map.set_Type(40, 76, 1);
	Map.set_Type(40, 77, 1);
	Map.set_Type(40, 78, 1);
	Map.set_Type(40, 79, 1);

	Map.set_Type(40, 111, 1);
	Map.set_Type(40, 112, 1);
	Map.set_Type(40, 113, 1);
	Map.set_Type(40, 114, 1);

	Map.set_Type(39, 146, 1);


	Map.set_Type(45, 111, 1);
	Map.set_Type(45, 112, 1);
	Map.set_Type(45, 113, 1);
	Map.set_Type(46, 112, 1);


	for (int cou_Y = 13; cou_Y < 19; cou_Y++)
	{
		Map.set_Type(45, cou_Y, 1);
		Map.set_Type(46, cou_Y, 1);
	}

	//Map.set_Type(5, 5, 1);
	//Map.set_Type(45, 45, 1);

	Map.add_Sensor(11, 30);
	Map.add_Sensor(11, 65);
	Map.add_Sensor(11, 100);
	Map.add_Sensor(11, 135);

	Map.add_Sensor(47, 11);


	Map.add_Actuator(51, 14, 750);
	Map.add_Actuator(51, 16, 750);

	Map.add_Actuator(50, 90, 0);
	Map.turn_Actuator_On(2);

	//Outside
	Map.add_Actuator(0, 0, 0);
	Map.add_Actuator(0, 25, 0);
	Map.add_Actuator(0, 50, 0);
	Map.add_Actuator(0, 75, 0);
	Map.add_Actuator(0, 100, 0);
	Map.add_Actuator(0, 125, 0);
	Map.add_Actuator(0, 150, 0);
	Map.add_Actuator(0, 174, 0);

	Map.turn_Actuator_On(3);
	Map.turn_Actuator_On(4);
	Map.turn_Actuator_On(5);
	Map.turn_Actuator_On(6);
	Map.turn_Actuator_On(7);
	Map.turn_Actuator_On(8);
	Map.turn_Actuator_On(9);
	Map.turn_Actuator_On(10);

	/*
	//Map.set_Temp(tmp_X, tmp_Y, tmp_Temp);
	for (int cou_Index = 0; cou_Index < 4; cou_Index++)
	{
		for (int cou_Index = 0; cou_Index < 75; cou_Index++)
		{
			Map.set_Temp(24, 24, 0);

			Map.update();
		}
		Map.view_Map();
	}*/

	while (1)
	{
		tmp_X = 0;
		tmp_Y = 0;
		tmp_Temp = 0;

		/*
		std::cout << "Enter Coord:";
		std::cout << "\n X:";
		std::cin >> tmp_X;
		std::cout << " Y:";
		std::cin >> tmp_Y;
		std::cout << " Enter Char: ";

		std::cin >> tmp_Temp;
		*/

		tmp_X = (rand() % 10) + 20;
		tmp_Y = (rand() % 10) + 20;
		tmp_Temp = float((rand() % 2) * 200);

		flg_Ranran = 0;
		//if (!(rand() % 5)) { flg_Ranran = 1; }
		//if (!flg_AC && !flg_Heater) { flg_Ranran = 1; }

		//Map.set_Temp(tmp_X, tmp_Y, tmp_Temp);
		for (int cou_Index = 0; cou_Index < 1; cou_Index++)
		{

			for (int cou_Index = 0; cou_Index < 25; cou_Index++)
			{
				tmp_Sensor = Map.get_Sensor_Data(2);
				if (flg_Ranran) { Map.set_Temp(tmp_X, tmp_Y, tmp_Temp); }

				if (tmp_Sensor > 102) { flg_AC = true; }
				if (tmp_Sensor < 98) { flg_Heater = true; }

				if (tmp_Sensor > 100) { flg_Heater = false; }
				if (tmp_Sensor < 100) { flg_AC = false; }

				if (flg_Heater) 
				{ 
					//Map.set_Temp(45, 45, 225); 
					Map.turn_Actuator_On(0);
					Map.turn_Actuator_On(1);
				}
				if (!flg_Heater) 
				{ 
					//Map.set_Temp(45, 45, 225); 
					Map.turn_Actuator_Off(0);
					Map.turn_Actuator_Off(1);
				}

				/*
				if (flg_AC) 
				{ 
					//Map.set_Temp(5, 5, 0); 
					Map.turn_Actuator_On(2);
					Map.turn_Actuator_On(3);
				}

				if (!flg_AC) 
				{ 
					//Map.set_Temp(5, 5, 0); 
					Map.turn_Actuator_Off(2);
					Map.turn_Actuator_Off(3);
				}*/

				//Map.set_Temp(30, 40, 125);

				Map.update();
			}
			/*
			std::cout << " Temp at Sensor (25, 25): " << tmp_Sensor;
			std::cout << " AC: " << flg_AC;
			std::cout << " Heat: " << flg_Heater;
			*/
			std::cout << "     ";
			std::cout << "     ";
			std::cout << "     ";
			std::cout << "     ";
			std::cout << "     ";
			std::cout << "     ";
			Map.view_Map();
		}
	}

}



















/*
Map.set_Type(23, 40, 1);
Map.set_Type(24, 40, 1);

Map.set_Type(26, 40, 1);
Map.set_Type(27, 40, 1);
*/

/*
for (int cou_X = 40; cou_X <= 50; cou_X++)
{
	Map.set_Type(22, cou_X, 0);

	Map.set_Type(25, cou_X, 0);

	Map.set_Type(28, cou_X, 0);
}

for (int cou_X = 0; cou_X < 10; cou_X++)
{
	Map.set_Type(25, cou_X, 0);
}

for (int cou_X = 30; cou_X < 74; cou_X++)
{
	Map.set_Type(25, cou_X, 0);
}

for (int cou_X = 0; cou_X < 55; cou_X++)
{
	Map.set_Type(cou_X, (cou_X + 70), 0);
}
for (int cou_X = 130; cou_X < 140; cou_X++)
{
	Map.set_Type(15, cou_X, 0);
}
for (int cou_X = 130; cou_X < 140; cou_X++)
{
	Map.set_Type(45, cou_X, 0);
}*/

/*
for (int cou_N = 0; cou_N < 100; cou_N++)
{
	int tmp_XX = (rand() % 30) + 15;
	int tmp_YY = (rand() % 5) + 130;

	Map.set_Type(tmp_XX, tmp_YY, 0);
}*/

/*
for (int cou_X = 0; cou_X < 60; cou_X++)
{
	Map.set_Type(10, cou_X, 0);
}

for (int cou_X = 10; cou_X < 75; cou_X++)
{
	Map.set_Type(20, cou_X, 0);
}

for (int cou_X = 0; cou_X < 60; cou_X++)
{
	Map.set_Type(30, cou_X, 0);
}

for (int cou_X = 10; cou_X < 75; cou_X++)
{
	Map.set_Type(40, cou_X, 0);
}
*/


























class c_Goblin_Map_Sim
{
	std::vector<std::vector<std::vector<c_Cell>>> Map;
	std::vector<std::vector<std::vector<c_Cell>>> Temp_Map;
	bool Current_Frame;
	bool Next_Frame;

	int Width;

	int Height;

	std::string Filename;

	int Tick;

	std::vector<c_Agent> The_Outsiders;

public:

	c_Goblin_Map_Sim(int p_Width, int p_Height)
	{
		Current_Frame = 0;
		Next_Frame = 1;

		Filename = "./Scripts/";

		new_Map(p_Width, p_Height);

		Tick = 0;
	}

	void init_Agents(int p_Count)
	{
		The_Outsiders.resize(p_Count * 3);

		for (int cou_Index = 0; cou_Index < p_Count; cou_Index++)
		{
			The_Outsiders[cou_Index].X = (rand() % Width);
			The_Outsiders[cou_Index].Y = (rand() % Height);
			The_Outsiders[cou_Index].Name = "Agent_" + std::to_string(cou_Index);
			The_Outsiders[cou_Index].Goal = 25000;
			The_Outsiders[cou_Index].Color = "blue";
		}

		for (int cou_Index = p_Count; cou_Index < (p_Count * 2); cou_Index++)
		{
			The_Outsiders[cou_Index].X = (rand() % Width);
			The_Outsiders[cou_Index].Y = (rand() % Height);
			The_Outsiders[cou_Index].Name = "Agent_" + std::to_string(cou_Index);
			The_Outsiders[cou_Index].Goal = 25000;
			The_Outsiders[cou_Index].Color = "green";
		}

		for (int cou_Index = (p_Count * 2); cou_Index < (p_Count * 3); cou_Index++)
		{
			The_Outsiders[cou_Index].X = (rand() % Width);
			The_Outsiders[cou_Index].Y = (rand() % Height);
			The_Outsiders[cou_Index].Name = "Agent_" + std::to_string(cou_Index);
			The_Outsiders[cou_Index].Goal = 25000;
			The_Outsiders[cou_Index].Color = "red";
		}
	}

	//B 70
	//G 70
	//R 50

	int get_Agent_Count()
	{
		return The_Outsiders.size();
	}

	int get_Width() { return Width; }
	int get_Height() { return Height; }

	void swap_Frame()
	{
		Current_Frame = !Current_Frame;
		Next_Frame = !Current_Frame;
		//std::cout << "\n[" << Current_Frame << " " << Next_Frame << "]";
	}

	void set_Agent_XY(int p_Agent, float p_X, float p_Y)
	{
		The_Outsiders[p_Agent].X = p_X;
		The_Outsiders[p_Agent].Y = p_Y;
	}

	void set_Agent_HP(int p_Agent, int p_HP)
	{
		The_Outsiders[p_Agent].HP = p_HP;
	}

	void new_Map(int p_Width, int p_Height)
	{
		Width = p_Width;
		Height = p_Height;

		std::cout << "\n Creating Map(" << Width << ", " << Height << ")";

		Map.resize(2);
		Temp_Map.resize(1);

		Map[0].resize(p_Width);
		Map[1].resize(p_Width);
		Temp_Map[0].resize(p_Width);
		for (int cou_X = 0; cou_X < p_Width; cou_X++)
		{
			Map[0][cou_X].resize(p_Height);
			Map[1][cou_X].resize(p_Height);
			Temp_Map[0][cou_X].resize(p_Height);
		}

		std::cout << "\n Finished Map";
	}

	void reset_Tick()
	{
		Tick = 0;
	}

	int get_Tick()
	{
		return Tick;
	}

	void set_Temp(int p_X, int p_Y, float p_Temp)
	{
		Map[Current_Frame][p_X][p_Y].Temp = p_Temp;
	}

	void set_Map_Temp(float p_Temp)
	{
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				set_Temp(cou_X, cou_Y, p_Temp);
			}
		}
	}

	void set_Type(int p_X, int p_Y, int p_Type)
	{
		Map[Current_Frame][p_X][p_Y].Type = p_Type;
		Map[Next_Frame][p_X][p_Y].Type = p_Type;
	}

	bool check_XY(int p_X, int p_Y)
	{
		if (p_X < 0) { return 0; }
		if (p_Y < 0) { return 0; }
		if (p_X >= Width) { return 0; }
		if (p_Y >= Height) { return 0; }

		if (Map[Current_Frame][p_X][p_Y].Type == 0) { return 0; }

		return 1;
	}

	void diffuse_Temp(int p_X, int p_Y)
	{
		float tmp_Total_Diff = 0.0;
		float tmp_Count = 0.0;
		float tmp_Current = 0.0;
		float tmp_Flow = 0.0;
		float tmp_Largest_Diff = 0.0;

		if (!check_XY(p_X, p_Y)) { return; }

		//Find the difference for each side. Average the lowest only.
		tmp_Current = Map[Current_Frame][p_X][p_Y].Temp;
		float tmp_Current_Diff[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		float tmp_Current_Diff_Weight[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		int tmp_XS[8] = { +1, 0, -1, 0, +1, -1, +1, -1 };
		int tmp_YS[8] = { 0, +1, 0, -1, +1, -1, -1, +1 };

		float tmp_Amount_To_Transfer = 0;


		for (int cou_I = 0; cou_I < 8; cou_I++)
		{
			int tmp_NX = p_X + tmp_XS[cou_I];
			int tmp_NY = p_Y + tmp_YS[cou_I];

			if (check_XY(tmp_NX, tmp_NY)) 
			{ 
				if ((Map[Current_Frame][tmp_NX][tmp_NY].Temp) < tmp_Current) 
				{ 
					tmp_Current_Diff[cou_I] = (tmp_Current - Map[Current_Frame][tmp_NX][tmp_NY].Temp); 
					if (tmp_Current_Diff[cou_I] > tmp_Largest_Diff) { tmp_Largest_Diff = tmp_Current_Diff[cou_I]; }
				}
			}
		}


		tmp_Total_Diff = 0;
		for (int cou_I = 0; cou_I < 8; cou_I++)
		{
			tmp_Total_Diff += tmp_Current_Diff[cou_I];
		}

		if (tmp_Total_Diff == 0) { return; }

		//In the future this will be used to create different materials and such.
		float tmp_Diffusion_Rate = 0.5;

		tmp_Amount_To_Transfer = tmp_Largest_Diff * tmp_Diffusion_Rate;

		
		std::ofstream tmp_SF;

		//Calculate weights
		std::cout << "\n\n Total_DIff: " << tmp_Total_Diff << " Largest_Diff: " << tmp_Largest_Diff << " Amount_To_Transfer: " << tmp_Amount_To_Transfer;
		for (int cou_I = 0; cou_I < 8; cou_I++)
		{
			tmp_Current_Diff_Weight[cou_I] = 0;
			if (tmp_Current_Diff[cou_I] != 0) 
			{
				tmp_Current_Diff_Weight[cou_I] = tmp_Current_Diff[cou_I] / tmp_Total_Diff;
			}
			std::cout << "\n   [" << cou_I << "] Diff: " << tmp_Current_Diff[cou_I] << " Weight: " << tmp_Current_Diff_Weight[cou_I] << " Amount: " << (tmp_Current_Diff_Weight[cou_I] * tmp_Amount_To_Transfer);
		}

		for (int cou_I = 0; cou_I < 8; cou_I++)
		{
			int tmp_NX = p_X + tmp_XS[cou_I];
			int tmp_NY = p_Y + tmp_YS[cou_I];
			float tmp_Current_Amount_To_Transfer = tmp_Amount_To_Transfer * tmp_Current_Diff_Weight[cou_I];

			if (tmp_Current_Diff[cou_I] != 0)
			{
				Map[Next_Frame][tmp_NX][tmp_NY].Temp += tmp_Current_Amount_To_Transfer;
				Map[Next_Frame][p_X][p_Y].Temp -= tmp_Current_Amount_To_Transfer;
			}
		}

	}

	void apply_Diffusion(int p_X, int p_Y)
	{
		Map[Next_Frame][p_X][p_Y].Temp += Temp_Map[0][p_X][p_Y].Temp;
	}

	float get_Temp(int p_X, int p_Y)
	{
		return Map[Current_Frame][p_X][p_Y].Temp;
	}

	void chill_Map(float p_Chill)
	{
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				Map[Current_Frame][cou_X][cou_Y].Temp -= p_Chill;
				if (Map[Current_Frame][cou_X][cou_Y].Temp < 0) { Map[Current_Frame][cou_X][cou_Y].Temp = 0; }
			}
		}
	}

	void update()
	{
		/*
		for (int cou_X = 0; cou_X < (Width); cou_X++)
		{
			for (int cou_Y = 0; cou_Y < (Height); cou_Y++)
			{
				//Map[Next_Frame][cou_X][cou_Y].Temp = Map[Current_Frame][cou_X][cou_Y].Temp;
				Map[Current_Frame][cou_X][cou_Y].Temp = 0;
				Map[Next_Frame][cou_X][cou_Y].Temp = 0;
				//Temp_Map[0][cou_X][cou_Y].Temp = 0;
			}
		}
		*/
		//set_Map_Temp(0);
		chill_Map(5);

		for (int cou_Diffuse_Run = 0; cou_Diffuse_Run < 10; cou_Diffuse_Run++)
		{
			for (int cou_Index = 0; cou_Index < The_Outsiders.size(); cou_Index++)
			{
				int tmp_OX = The_Outsiders[cou_Index].X;
				int tmp_OY = The_Outsiders[cou_Index].Y;

				int tmp_Dist = 5;
				int tmp_Charge = 10;

				if (check_XY(tmp_OX, tmp_OY))
				{
					if (The_Outsiders[cou_Index].Color == "blue")
					{
						if (check_XY(tmp_OX + tmp_Dist, tmp_OY + tmp_Dist))
						{
							Map[Current_Frame][tmp_OX + tmp_Dist][tmp_OY + tmp_Dist].Temp += tmp_Charge;
						}
						if (check_XY(tmp_OX - tmp_Dist, tmp_OY - tmp_Dist))
						{
							Map[Current_Frame][tmp_OX - tmp_Dist][tmp_OY - tmp_Dist].Temp += tmp_Charge;
						}
						//Map[Current_Frame][tmp_OX][tmp_OY].Temp = 50 - tmp_Charge;
					}

					tmp_Dist = 10;
					tmp_Charge = 20;
					if (The_Outsiders[cou_Index].Color == "green")
					{
						if (check_XY(tmp_OX + tmp_Dist, tmp_OY))
						{
							Map[Current_Frame][tmp_OX + tmp_Dist][tmp_OY].Temp += tmp_Charge;
						}
						if (check_XY(tmp_OX - tmp_Dist, tmp_OY))
						{
							Map[Current_Frame][tmp_OX - tmp_Dist][tmp_OY].Temp += tmp_Charge;
						}
						//Map[Current_Frame][tmp_OX][tmp_OY].Temp = 50 - tmp_Charge;
					}

					tmp_Dist = 15;
					tmp_Charge = 30;
					if (The_Outsiders[cou_Index].Color == "red")
					{
						if (check_XY(tmp_OX + tmp_Dist, tmp_OY - tmp_Dist))
						{
							Map[Current_Frame][tmp_OX + tmp_Dist][tmp_OY - tmp_Dist].Temp += tmp_Charge;
						}
						if (check_XY(tmp_OX - tmp_Dist, tmp_OY + tmp_Dist))
						{
							Map[Current_Frame][tmp_OX - tmp_Dist][tmp_OY + tmp_Dist].Temp += tmp_Charge;
						}
						//Map[Current_Frame][tmp_OX][tmp_OY].Temp = 50 - tmp_Charge;
					}
				}
			}

			for (int cou_X = 0; cou_X < (Width); cou_X++)
			{
				for (int cou_Y = 0; cou_Y < (Height); cou_Y++)
				{
					diffuse_Temp(cou_X, cou_Y);
					//GPT_diffusion(cou_X, cou_Y);
				}
			}
		}

		/*
		for (int cou_X = 0; cou_X < (Width); cou_X++)
		{
			std::cout << "\n";
			for (int cou_Y = 0; cou_Y < (Height); cou_Y++)
			{
				if (Map[Next_Frame][cou_X][cou_Y].Temp < 10) { std::cout << " "; }
				if (Map[Next_Frame][cou_X][cou_Y].Temp < 100) { std::cout << " "; }

				std::cout << int(Map[Next_Frame][cou_X][cou_Y].Temp) << " ";
			}
		}
		
		for (int cou_X = 0; cou_X < (Width); cou_X++)
		{
			for (int cou_Y = 0; cou_Y < (Height); cou_Y++)
			{
				apply_Diffusion(cou_X, cou_Y);
			}
		}
		*/

		float tmp_Temps[5] = { -99, -99, -99, -99, -99 };

		//std::cout << "\n";
		for (int cou_Index = 0; cou_Index < The_Outsiders.size(); cou_Index++)
		{

			tmp_Temps[0] = -99;
			tmp_Temps[1] = -99;
			tmp_Temps[2] = -99;
			tmp_Temps[3] = -99;
			tmp_Temps[4] = -99;

			if (check_XY(The_Outsiders[cou_Index].X - 1, The_Outsiders[cou_Index].Y)) { tmp_Temps[0] = Map[Current_Frame][The_Outsiders[cou_Index].X - 1][The_Outsiders[cou_Index].Y].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y + 1)) { tmp_Temps[1] = Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y + 1].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X + 1, The_Outsiders[cou_Index].Y)) { tmp_Temps[2] = Map[Current_Frame][The_Outsiders[cou_Index].X + 1][The_Outsiders[cou_Index].Y].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y - 1)) { tmp_Temps[3] = Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y - 1].Temp; }
			if (check_XY(The_Outsiders[cou_Index].X, The_Outsiders[cou_Index].Y)) { tmp_Temps[4] = Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp; }
			//std::cout << "\n MT " << Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp;
			//std::cout << "\n TM " << tmp_Temps[4];
			The_Outsiders[cou_Index].update(tmp_Temps, Width, Height);
			The_Outsiders[cou_Index].HP = 1000;

			float tmp_Temperature = 0.1 * ((rand() % 3) - 1);
			//std::cout << "\n\n Temp: " << tmp_Temperature;  std::cout << "\n     ..X: " << The_Outsiders[cou_Index].X; std::cout << "\n     ..Y: " << The_Outsiders[cou_Index].Y;

			tmp_Temperature = 0.1 * ((rand() % 3) - 1);
			The_Outsiders[cou_Index].X -= tmp_Temperature + (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_X) * (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_X);

			if (The_Outsiders[cou_Index].X >= Width) { The_Outsiders[cou_Index].X = Width - 1; }
			if (The_Outsiders[cou_Index].X <= 0) { The_Outsiders[cou_Index].X = 1; }

			tmp_Temperature = 0.1 * ((rand() % 3) - 1);
			The_Outsiders[cou_Index].Y -= tmp_Temperature + (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_Y) * (The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude_Y);

			if (The_Outsiders[cou_Index].Y >= Height) { The_Outsiders[cou_Index].Y = Height - 1; }
			if (The_Outsiders[cou_Index].Y <= 0) { The_Outsiders[cou_Index].Y = 1; }

			//std::cout << "\n     X..: " << The_Outsiders[cou_Index].X; std::cout << "\n     Y..: " << The_Outsiders[cou_Index].Y;


			//if (The_Outsiders[cou_Index].flg_Alive == false) { continue; }

			/*
			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 0)
			{
				The_Outsiders[cou_Index].Y += The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].Y >= Height) { The_Outsiders[cou_Index].Y = Height - 1; }
			}

			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 1)
			{
				The_Outsiders[cou_Index].X += The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].X >= Width) { The_Outsiders[cou_Index].X = Width - 1; }
			}

			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 2)
			{
				The_Outsiders[cou_Index].Y -= The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].Y <= 0) { The_Outsiders[cou_Index].Y = 1; }
			}

			if (Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Direction == 3)
			{
				The_Outsiders[cou_Index].X -= The_Outsiders[cou_Index].Speed * Map[Current_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Magnitude;
				if (The_Outsiders[cou_Index].X <= 0) { The_Outsiders[cou_Index].X = 1; }
			}*/

			//float tmp_Temp = Map[Next_Frame][The_Outsiders[cou_Index].X][The_Outsiders[cou_Index].Y].Temp;

		}


		//B 70
		//G 70
		//R 50


		swap_Frame();

		Tick++;
	}

	void output_Agents()
	{
		std::cout << "\n";
		for (int cou_Index = 0; cou_Index < The_Outsiders.size(); cou_Index++)
		{
			The_Outsiders[cou_Index].output();
		}
	}

	void view_Map()
	{
		std::string tmp_Out = "\n";
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			tmp_Out += "\n[";
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				//std::cout << "[" << Map[Current_Frame][cou_X][cou_Y].Temp << "]";

				if (Map[Current_Frame][cou_X][cou_Y].Type == 0)
				{
					tmp_Out += char(178);
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Type == 2)
				{
					tmp_Out += 'S';
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Type == 3)
				{
					tmp_Out += 'A';
					continue;
				}
				if (char(Map[Current_Frame][cou_X][cou_Y].Temp) > 20)
				{
					tmp_Out += char(Map[Current_Frame][cou_X][cou_Y].Temp);
				}
				else
				{
					tmp_Out += " ";
				}
			}
			tmp_Out += "]";
		}
		//tmp_Out += "\n";

		std::cout << tmp_Out;
	}

	void view_Map_Raw()
	{
		std::string tmp_Out = "\n";
		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			tmp_Out += "\n[";
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				//std::cout << "[" << Map[Current_Frame][cou_X][cou_Y].Temp << "]";

				if (Map[Current_Frame][cou_X][cou_Y].Type == 0)
				{
					tmp_Out += char(178);
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Type == 2)
				{
					tmp_Out += 'S';
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Type == 3)
				{
					tmp_Out += 'A';
					continue;
				}
				if (Map[Current_Frame][cou_X][cou_Y].Temp < 10) { tmp_Out += " "; }
				if (Map[Current_Frame][cou_X][cou_Y].Temp < 100) { tmp_Out += " "; }
				tmp_Out += " _" + std::to_string(int(Map[Current_Frame][cou_X][cou_Y].Temp)) + "_";
			}
			tmp_Out += "]";
		}
		//tmp_Out += "\n";

		std::cout << tmp_Out;
	}


	void write_Map_Tiles(std::string p_FName)
	{
		std::ofstream SF;
		std::string tmp_FName = p_FName + "." + get_Padded(10, Tick) + ".MAP.ssv";
		SF.open(tmp_FName, std::ios::trunc);

		std::ofstream SFL;
		std::string tmp_LFName = p_FName + "." + get_Padded(10, Tick) + ".MAP.lbl";
		SFL.open(tmp_LFName, std::ios::trunc);

		std::vector<std::vector<float>> tmp_Map;
		tmp_Map.resize(Width);

		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			tmp_Map[cou_X].resize(Height);
		}

		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				tmp_Map[cou_X][cou_Y] = Map[Current_Frame][cou_X][cou_Y].Temp;
			}
		}

		for (int cou_Agent = 0; cou_Agent < The_Outsiders.size(); cou_Agent++)
		{
			if (The_Outsiders[cou_Agent].flg_Alive == 0) { continue; }

			SFL << "A" << cou_Agent << " " << The_Outsiders[cou_Agent].Y << " " << The_Outsiders[cou_Agent].X << " 4 " << The_Outsiders[cou_Agent].Color << " +\n";
		}

		for (int cou_X = 0; cou_X < Width; cou_X++)
		{
			if (cou_X > 0)
			{
				SF << "\n";
			}
			SF << Tick << " ";
			for (int cou_Y = 0; cou_Y < Height; cou_Y++)
			{
				SF << tmp_Map[cou_X][cou_Y] << " ";
			}
		}

		SF.close();
		SFL.close();
	}


	void draw_Building(int p_X0, int p_Y0, int p_X1, int p_Y1)
	{
		for (int cou_X = p_X0; cou_X <= p_X1; cou_X++)
		{
			set_Type(cou_X, p_Y0, 0);
			set_Type(cou_X, p_Y1, 0);
		}

		for (int cou_Y = p_Y0; cou_Y <= p_Y1; cou_Y++)
		{
			set_Type(p_X0, cou_Y, 0);
			set_Type(p_X1, cou_Y, 0);
		}

		//p_Map->set_Type(23, 20, 1);
	}
};



void run_Goblin_World()
{
	c_Goblin_Map_Sim Map(128, 128);

	Map.set_Map_Temp(70);

	Map.set_Type(0, 2, 0);
	Map.set_Type(1, 0, 0);
	Map.set_Type(1, 1, 0);
	Map.set_Type(1, 2, 0);

	Map.init_Agents(15);

	float tmp_Temp = 45;
	Map.set_Temp(1, 1, 60);
	int tmp_Step = 10;

	while (1)
	{
		for (int cou_I = 0; cou_I < 1000; cou_I++)
		{
			for (int cou_S = 0; cou_S < tmp_Step; cou_S++)
			{
				//Map.set_Temp(1, 1, tmp_Temp);
				//Map.set_Temp(10, 40, 0);
				//Map.set_Temp(40, 40, 100);
				Map.update();
			}
			Map.view_Map();


			Map.set_Temp(0, 0, 0);
			Map.set_Temp(0, 1, 200);

			Map.write_Map_Tiles("GaiaTesting/Yoho");
			//Map.view_Map_Raw();
			std::cout << "\n_______________________________________\n\n";
		}
	}

}