#include "DailyTransaction.h"
#include "EquityPriceGenerator.h"

#include <vector>
#include <algorithm>		// std::shuffle
#include <numeric>			// std::iota
#include <random>			// Mersenne Twister 64
#include <iostream>

using RealVector = std::vector<double>;
using IntVector = std::vector<int>;
using TxnVector = std::vector<DailyTransaction>;

TxnVector generateSimulatedPath(int days, double mktPrice, double drift, double volatility, int seed);
void withoutReplacementTests(int days, double mktPrice, double drift, double volatility,
							 int seed, int numScenarios);
void withReplacementTests(int days, double mktPrice, double drift, double volatility,
						  int seed, int numScenarios);
void checkResults(const std::vector<TxnVector>& scenarios, bool replacement);

int main()
{
	// Number of days = days *in addition* to the start date:
	withoutReplacementTests(7, 100.0, 0.15, 0.25, 106, 15);
	withReplacementTests(7, 100.0, 0.15, 0.25, 106, 15);

}

TxnVector generateSimulatedPath(int days, double mktPrice, double drift, double volatility, int seed)
{
	// This is a temporary measure to replicate a single asset strategy of daily trades.
	// In the final product, this will come from the actual path resulting from a strategy.
	EquityPriceGenerator epg(mktPrice, days, 1.0, drift, volatility);
	auto prices = epg(seed);
	TxnVector txns;

	for (auto iter = prices.begin(); iter != prices.end() - 1; ++iter)
	{
		auto startPrice = *iter;
		auto endPrice = *(++iter);
		DailyTransaction dailyTxn(startPrice, endPrice);
		--iter;
		txns.push_back(dailyTxn);
	}

	return txns;
}

void withoutReplacementTests(int days, double mktPrice, double drift, double volatility,
							 int seed, int numScenarios)
{
	auto txns = generateSimulatedPath(days, mktPrice, drift, volatility, seed);

	// This is a contrived way of setting a different seed for 
	// each scenario.  There are more robust ways to do this if desired.
	IntVector seeds(numScenarios);
	int initSeed = 0;
	std::iota(seeds.begin(), seeds.end(), initSeed);

	// A vector container of individual vectors of shuffled transactions:
	std::vector<TxnVector> scenarios;

	for (const int& seed : seeds)
	{
		auto txnsCopy = txns;		// Requires copy -- is there a better way, or does it really matter? (Check)
		std::shuffle(txnsCopy.begin(), txnsCopy.end(), std::mt19937_64(seed));
		scenarios.push_back(txnsCopy);
	}

	auto row = scenarios.at(0);

	// Then, do the analysis on the set of randomised simulations in scenarios.
	// checkResults(.) is just a first test that will not be included in the final model:
	checkResults(scenarios, false);
}

void withReplacementTests(int days, double mktPrice, double drift, double volatility,
	int seed, int numScenarios)
{
	auto txns = generateSimulatedPath(days, mktPrice, drift, volatility, seed);

	// This is a contrived way of setting a different seed for 
	// each scenario.  There are more robust ways to do this, but 
	// we can deal with this later.
	IntVector seeds(numScenarios);
	int initSeed = 0;
	std::iota(seeds.begin(), seeds.end(), initSeed);

	// In this case, we will sample *with* replacement.  We will sample the integer
	// indices of the transactions {0, 1, ..., n-1}, and then retrieve these transactions
	// and place them in a vector corresponding to the simulation being constructed.  
	// To do this, we'll use the Mersenne Twister engine and the uniform integer distribution
	// transformation in the Standard Library:

	auto generateRandIndices = [&txns](int seed)
	{
		std::mt19937_64 mtEngine(seed);
		int numTransactions = static_cast<int>(txns.size());
		std::uniform_int_distribution<> ud(0, numTransactions - 1);
		IntVector indices;
		for (int i = 0; i < numTransactions; ++i)
		{
			indices.push_back(ud(mtEngine));
		}

		return indices;
	};

	auto generateScenario = [txns](const IntVector& indices)
	{
		TxnVector withReplTransactions;
		for (int i = 0; i < indices.size(); ++i)
		{
			withReplTransactions.push_back(txns.at(indices.at(i)));
		}
		return withReplTransactions;
	};


	// A vector container of individual vectors of randomised transactions with replacement:
	std::vector<TxnVector> scenarios;
	for (const int& seed : seeds)
	{
		auto indices = generateRandIndices(seed);
		auto txnsWithRepl = generateScenario(indices);
		scenarios.push_back(txnsWithRepl);
	}

	// Again, do the analysis on the set of randomised simulations in scenarios, this time with replacement.
	// checkResults(.) is just a first test that will not be included in the final model:
	checkResults(scenarios, true);

}

void checkResults(const std::vector<TxnVector>& scenarios, bool replacement)
{
	// (x > y) ? z : y;
	std::cout << "***** Check results of generated scenarios *****" << "\n";
	std::cout << "Scenarios generated " << ((replacement) ? "with replacement." : "without replacement.") << "\n";
	std::cout << "Number of scenarios generated: " << scenarios.size() << "\n";
	std::cout << "Number of transactions in each scenario: " << scenarios.at(0).size() << "\n";
	std::cout << "Matrix of daily transaction returns: " << "\n";
	
	auto scenarioTransactionReturns = [](const TxnVector& tv)
	{
		for (const auto& txn : tv)
		{
			std::cout << txn() << "\t";
		}

		std::cout << "\n";
	};
	
	for (const auto& scenario : scenarios)
	{
		scenarioTransactionReturns(scenario);
	}

	std::cout << "\n\n";
}

