#ifndef DAILY_TRANSACTION_H
#define DAILY_TRANSACTION_H

class DailyTransaction
{
public:
	DailyTransaction(double startPrice, double endPrice) :
		startPrice_(startPrice), endPrice_(endPrice) {};

	// Returns simple daily return:
	double operator()() const
	{
		return (endPrice_ - startPrice_) / startPrice_;
	};

	// We could, if desired, and if data were available, include
	// other accessor functions for things like MAE etc.

private:
	double startPrice_;
	double endPrice_;
};


#endif // !DAILY_TRANSACTION_H

