
#include "GeneticAlgorithm.h"
#include <algorithm>
#include <iostream>
#include <ctime>
#include <limits>

//zwraca true je�li first jest korzystniejszym rozwiazaniem niz second
struct PathComparator {
	bool operator() (std::pair<std::vector<int>, int> first, std::pair<std::vector<int>, int> second)
	{
		return (first.second < second.second);
	}
};

GeneticAlgorithm::GeneticAlgorithm(TravelingSalesmanProblem& t) :
tsp(t)
{
}


GeneticAlgorithm::~GeneticAlgorithm()
{
}

//przeprowadzenie ca�ego algorytmu
void GeneticAlgorithm::startAlgorithm(int population_size, int generation_count, float mutation_probability)
{
	srand(time(NULL));

	std::pair<std::vector<int>, int> first_solution, new_solution;
	population.clear();

	//pierwsze rozwi�zanie wyznaczane jest losowo
	first_solution.first = generateRandomSolution();
	first_solution.second = countCost(first_solution.first);
	population.push_back(first_solution);

	//na pocz�tku do populacji trafiaj� rozwi�zania z s�siedztwa typu swap pierwszego losowego rozwi�zania 
	for (int i = 1; i < population_size; i++)
	{
		new_solution.first = first_solution.first;
		swapTwoRandomCities(new_solution.first);
		new_solution.second = countCost(new_solution.first);
		//do populacji trafiaj� tylko unikalne rozwi�zania
		if (solutionIsUnique(new_solution, population))
		{
			population.push_back(new_solution);
		}
		else
		{
			i--;
		}
		
	}
	PathComparator comp;
	//rozwi�zania s� sortowane od najlepszego do najgorszego
	std::sort(population.begin(), population.end(), comp);

	std::vector<std::pair<std::vector<int>, int> > next_generation;
	//ewolucja populacji przez wyznaczon� ilo�� pokole�
	for (int i = 0; i < generation_count; i++)
	{
		//kolejne pokolenie wyznaczone przez krzy�owanie typu PMX
		next_generation = crossoverPMX(population_size, mutation_probability);

		//do populacji trafiaj� najlepsze rozwi�zania z starej populacji i nowego pokolenia
		population.insert(population.end(), next_generation.begin(), next_generation.end());
		std::sort(population.begin(), population.end(), comp);
		population.resize(population_size);
	}

	the_best_solution = population[0].first;
	min_cost = population[0].second;
}

//wyznaczanie nastepnego pokolenia
std::vector<std::pair<std::vector<int>, int> > GeneticAlgorithm::crossoverPMX(int population_size, float mutation_probability)
{
	std::vector<std::pair<std::vector<int>, int> > next_generation;
	std::pair<std::vector<int>, int> first_parent, second_parent, first_child, second_child;
	int first_cross_point, second_cross_point;
	//nowe pokolenie jest niemniejsze ni� populacja
	while (next_generation.size() < population_size)
	{
		//wybranie pary rozwi�za� metod� ruletki do krzy�owania
		first_parent = population[rand()%population.size()];
		second_parent = population[rand() % population.size()];				

		//wszystkie pola w �cie�kach potomstwa wype�niam -1, wiem, �e miasto nie mo�e mie� takiego numeru
		first_child.first.resize(tsp.getCitiesCount(), -1);
		second_child.first.resize(tsp.getCitiesCount(), -1);

		//losowo s� wybierane punkty krzy�owania
		first_cross_point = rand() % tsp.getCitiesCount();
		do
		{
			second_cross_point = rand() % tsp.getCitiesCount();
		} while (first_cross_point == second_cross_point);
		

		//first_cross_point musi by� bli�ej pocz�tku vectora
		if (first_cross_point > second_cross_point)
		{
			int buffor = first_cross_point;
			first_cross_point = second_cross_point;
			second_cross_point = buffor;
		}

		//do potomk�w trafiaj� na krzy� obszary krzy�owania
		for (int j = first_cross_point; j <= second_cross_point; j++)
		{
			first_child.first[j] = second_parent.first[j];
			second_child.first[j] = first_parent.first[j];
		}

		//uzupe�nienie reszty �cie�ki
		for (int j = 0; j < tsp.getCitiesCount(); j++)
		{
			//pomijam obszar krzy�owania
			if (j == first_cross_point)
			{
				j = second_cross_point;
				continue;
			}
				
			//dodanie odpowiedniego miasta na j-tej pozycji
			addCorrectCityAt(first_child, first_parent, j, first_cross_point, second_cross_point);
			

			addCorrectCityAt(second_child, second_parent, j, first_cross_point, second_cross_point);

		}

		//mutacje potomk�w zachodz� z okre�lonym prawdopodobie�stwem
		mutate(first_child, mutation_probability);
		mutate(second_child, mutation_probability);

		//do nast�pnego pokolenia trafiaj� tylko rozwi�zania nie znajduj�ce si� w tym pokoleniu i w populacji rodzicielskiej
		first_child.second = countCost(first_child.first);

		second_child.second = countCost(second_child.first);
		//if (solutionIsUnique(first_child, next_generation) && solutionIsUnique(first_child, population)) next_generation.push_back(first_child);
		//if (solutionIsUnique(second_child, next_generation) && solutionIsUnique(second_child, population)) next_generation.push_back(second_child);

		next_generation.push_back(first_child);
		next_generation.push_back(second_child);
		first_child.first.clear();
		second_child.first.clear();
	}
	return next_generation;
}

void GeneticAlgorithm::addCorrectCityAt(std::pair<std::vector<int>, int> &child, std::pair<std::vector<int>, int> const &parent, int position, int first, int second)
{
	//wska�nik przesuni�cia w strefie krzy�owania
	int shift = 0;
	//szeroko�� przedzia�u krzy�owania
	int interval = second - first + 1;

	//sprawdzenie, czy elemnt, kt�ry aktualnie ma by� dodany, nie znajduje si� ju� w rozwi�zaniu(by� w obszarze krzy�owania)
	int found_index = std::find(child.first.begin(),
		child.first.end(),
		parent.first[position])
		- child.first.begin();
	//je�li nie znaleziono elementu w �cie�ce
	if (found_index == child.first.size())
	{
		child.first[position] = parent.first[position];
	}
	else
	{
		//wyznaczenie elementu, kt�ry nale�y dopisa� ze strefy krzy�owania drugiego osobnika
		while (std::find(child.first.begin(), child.first.end(), parent.first[first + (found_index - first + shift) % interval])
			!= child.first.end())
		{
			shift++;
		}
		child.first[position] = parent.first[first + (found_index - first + shift) % interval];
	}
}

//mutacja rozwi�zania
void GeneticAlgorithm::mutate(std::pair<std::vector<int>, int>& speciman, float probability)
{
	//je�li zostanie wylosowane prawdopodobie�stwo mniejsze od podanego, nast�puje mutacja
	if ((float)rand() / RAND_MAX < probability)
	{
		//wylosowana zostaje ilo�� ruch�w swap w mutacji, maksymalnie wielko��_populacji/4
		int swap_number = rand() % population.size()/4;
		for (int i = 0; i <= swap_number; i++)
		{
			swapTwoRandomCities(speciman.first);
		}
	}
}

//metoda zwraca indeks osobnika w populacji, zgodnie z zasad� ruletki, lepsze rozwi�zania s� cz�ciej zwracane
int GeneticAlgorithm::rouletteWheelSelection()
{
	//licz� sum� wszystkich koszt�w wyznaczonych �cie�ek
	int costs_sum = 0;
	for each (std::pair<std::vector<int>, int> specimen in population)
	{
		costs_sum += specimen.second;
	}

	int rand_result = rand() % costs_sum;

	int chosen_spiecman = -1;

	while (rand_result >= 0)
	{
		chosen_spiecman++;
		rand_result -= population[population.size() - chosen_spiecman-1].second;
	}
	return chosen_spiecman;
}

//sprawdzenie czy dane rozwi�zanie jest unikalne
bool GeneticAlgorithm::solutionIsUnique(std::pair<std::vector<int>, int> new_solution, std::vector<std::pair<std::vector<int>, int> > population)
{
	for each (std::pair<std::vector<int>, int> solution in population)
	{
		//je�li koszty s� r�ne, wiadomo, ze rozwi�zania s� r�ne
		if (new_solution.second == solution.second)
		{
			//wyznaczenie wsp�lnego pocz�tku permutacji
			int common_begin = 0;
			while (new_solution.first[0] != solution.first[common_begin])
			{
				common_begin++;
			}
			int index = 0;
			int equal_cities = 0;
			for (int i = 0; i < new_solution.first.size(); i++)
			{
				index = (common_begin + i) % new_solution.first.size();
				//zliczanie wsp�lnych miast w �cie�ce
				if (new_solution.first[i] == solution.first[index])
				{
					equal_cities++;
				}
				else
					break;
			}
			//je�li wszystkie miasta s� takie same, to rozwi�zanie nie jest unikalne
			if (equal_cities == new_solution.first.size())
			{
				return false;
			}
		}
	}
	return true;
}

//zwraca losow� permutacje miast
std::vector <int> GeneticAlgorithm::generateRandomSolution()
{
	std::vector <int> solution;
	for (int i = 0; i < tsp.getCitiesCount(); i++)
	{
		solution.push_back(i);
	}
	std::random_shuffle(solution.begin(), solution.end());
	return solution;
}

//liczy koszt przej�cia podanej �cie�ki
int GeneticAlgorithm::countCost(std::vector<int> path)
{
	int cost = 0;
	for (std::size_t i = 0; i < path.size() - 1; i++)
	{
		cost += tsp.getEdgeCost(path[i], path[i + 1]);
	}
	cost += tsp.getEdgeCost(path[path.size() - 1], path[0]);

	return cost;
}

//zamienia miejscami dwa losowe miasta
void GeneticAlgorithm::swapTwoRandomCities(std::vector<int>& path)
{
	int firstPosition = rand() % path.size();

	int secondPosition;
	do {
		secondPosition = rand() % path.size();
	} while (firstPosition == secondPosition);

	int buffor = path.at(firstPosition);
	path.at(firstPosition) = path.at(secondPosition);
	path.at(secondPosition) = buffor;
}

//zwraca rozwi�zanie w formie do wy�wietlenia
std::string GeneticAlgorithm::getSolutionToString(std::vector<int> path, int cost)
{
	std::string str = "";
	str.append("Koszt przejscia sciezki:\n");
	str.append(std::to_string(cost));
	str.append("\nSciezka:\n");
	for (std::size_t i = 0; i < path.size(); i++)
	{
		str.append(std::to_string(path[i]));
		str.append(" ");
	}

	return str;
}

void GeneticAlgorithm::startAlgorithmStepByStep(int population_size, int generation_count, float mutation_probability)
{
	srand(time(NULL));

	std::pair<std::vector<int>, int> first_solution, new_solution;
	population.clear();

	//pierwsze rozwi�zanie wyznaczane jest losowo
	first_solution.first = generateRandomSolution();
	first_solution.second = countCost(first_solution.first);
	population.push_back(first_solution);

	std::cout << "Ilosc pokolen: " << generation_count << " Rozmiar populacji: " << population_size << std::endl;
	std::cout << "Generowanie poczatkowego pokolenia" << std::endl;
	//na pocz�tku do populacji trafiaj� rozwi�zania z s�siedztwa typu swap pierwszego losowego rozwi�zania 
	for (int i = 1; i < population_size; i++)
	{
		new_solution.first = first_solution.first;
		swapTwoRandomCities(new_solution.first);
		new_solution.second = countCost(new_solution.first);
		//do populacji trafiaj� tylko unikalne rozwi�zania
		if (solutionIsUnique(new_solution, population))
		{
			population.push_back(new_solution);
		}
		else
		{
			i--;
		}

	}
	PathComparator comp;
	//rozwi�zania s� sortowane od najlepszego do najgorszego
	std::cout << "Sortowanie pokolenia" << std::endl;
	std::sort(population.begin(), population.end(), comp);

	std::vector<std::pair<std::vector<int>, int> > next_generation;
	//ewolucja populacji przez wyznaczon� ilo�� pokole�
	for (int i = 0; i < generation_count; i++)
	{
		std::cout << "Wyznaczanie kolejnego pokolenia" << std::endl;
		//kolejne pokolenie wyznaczone przez krzy�owanie typu PMX
		next_generation = crossoverPMX(population_size, mutation_probability);

		//do populacji trafiaj� najlepsze rozwi�zania z starej populacji i nowego pokolenia
		population.insert(population.end(), next_generation.begin(), next_generation.end());
		std::sort(population.begin(), population.end(), comp);
		population.resize(population_size);
	}

	the_best_solution = population[0].first;
	min_cost = population[0].second;
}

