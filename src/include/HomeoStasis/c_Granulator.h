
//This class handles data granulation
//You must register the smallest (the goal/optimal) range first, then the bigger getting progressively further from the goal. It checks them iteratively.
//For example, if you have Temperature and the goal is to keep it at 75 (green), 5 degrees either way is not good but not catastrophic (yellow), and any more than that is catastrophic (red)
//you may have this setup:
// add_Var(74.9, 75.1) //The goal, always first
// add_Var(69.9, 75.1) //The yellow range
// add_Var(0.0, 100.0) //The red, here 0-100 is the range of the thermometer.
//The result depends on whether it is above or below the mid if within boundaries. If above, positive 1, if below, -1. The number from registering granulations is used here, it is the position of that granulation * result.
class c_Granulator
{
public:

	double * Top;
	double * Bottom;
	double * Mid;
	int Count;

	c_Granulator()
	{
		Top = NULL;
		Bottom = NULL;
		Mid = NULL;
		Count = 0;
	}

	void add_Var(double p_Bottom, double p_Top)
	{
		double* tmp_Top;
		double* tmp_Bottom;
		double* tmp_Mid;

		tmp_Top = new double[Count + 1];
		tmp_Bottom = new double[Count + 1];
		tmp_Mid = new double[Count + 1];

		for (int cou_Index = 0; cou_Index < Count; cou_Index++)
		{
			tmp_Top[cou_Index] = Top[cou_Index];
			tmp_Bottom[cou_Index] = Bottom[cou_Index];
			tmp_Mid[cou_Index] = Mid[cou_Index];
		}

		if (Top != NULL)
		{
			delete[] Top; Top = NULL;
			delete[] Bottom; Bottom = NULL;
			delete[] Mid; Mid = NULL;
		}

		Top = new double[Count + 1];
		Bottom = new double[Count + 1];
		Mid = new double[Count + 1];

		for (int cou_Index = 0; cou_Index < Count; cou_Index++)
		{
			Top[cou_Index] = tmp_Top[cou_Index];
			Bottom[cou_Index] = tmp_Bottom[cou_Index];
			Mid[cou_Index] = tmp_Mid[cou_Index];
		}

		Top[Count] = p_Top;
		Bottom[Count] = p_Bottom;
		Mid[Count] = ((p_Top - p_Bottom) / 2) + p_Bottom;

		if (tmp_Top != NULL) { delete[] tmp_Top; tmp_Top = NULL; }
		if (tmp_Bottom != NULL) { delete[] tmp_Bottom; }
		if (tmp_Mid != NULL) { delete[] tmp_Mid; }

		Count++;
	}

	double get_Value(int p_Var, double p_Value)
	{
		//Range goals
		double tmp_Value = 123;

		if ((p_Value >= Bottom[p_Var]) && (p_Value <= Mid[p_Var])) { tmp_Value = -1.0; }
		//if ((p_Value >= Bottom[p_Var]) && (p_Value <= Top[p_Var])) { tmp_Value = 0.0; }
		if ((p_Value <= Top[p_Var]) && (p_Value > Mid[p_Var])) { tmp_Value = 1.0; }

		//std::cout << "\n Goal_Index: " << p_Goal;
		//std::cout << " State: >" << tmp_State << "< tmp_Target_Goal_State: >" << tmp_Target_Goal_State_Low << "< >" << tmp_Target_Goal_State_High << "< Delta: >" << tmp_Delta << "<";

		return tmp_Value;
	}

	//This gets the granulated value by checking each range, the result is the index of the range found times the direction, -1 || 1
	//To prevent problems you should always set the largest range beyond what is possible to be passed.
	double get_Granulated(double p_Value)
	{
		double tmp_Val = 0.0;
		for (int cou_Index = 0; cou_Index < Count; cou_Index++)
		{
			tmp_Val = get_Value(cou_Index, p_Value);

			//If the range is found.
			if (tmp_Val != 123)
			{
				return (tmp_Val * cou_Index);
			}
		}

		//If you find this value then it means the input is out of bounds.
		return -0.123456;
	}
};
