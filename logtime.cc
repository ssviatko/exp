#include <iostream>

#include <time.h>
#include <math.h>

class logtime {
public:
	logtime(double a_initial, double a_maximum, double a_multiplier)
	: m_time(a_initial),
	  m_initial(a_initial),
	  m_maximum(a_maximum),
	  m_multiplier(a_multiplier)
	{ }
	~logtime() { }
	void increment()
	{
		m_time *= m_multiplier;
		if (m_time > m_maximum)
			m_time = m_maximum;
	} // next time increment please
	void logsleep(); // sleep for current time increment
	double time() { return m_time; }
	double initial() { return m_initial; }
	double maximum() { return m_maximum; }
	double multiplier() { return m_multiplier; }
protected:
	double m_time;
	double m_initial;
	double m_maximum;
	double m_multiplier;
};

void logtime::logsleep()
{
	struct timespec l_tlog, l_tlog_rem;

	double l_int;
	double l_frac = modf(m_time, &l_int);

	int l_nsec = (int)(l_frac * 1000000000.0);
	int l_sec = (int)l_int;

        l_tlog.tv_nsec = l_nsec;
        l_tlog.tv_sec = l_sec;

	nanosleep(&l_tlog, &l_tlog_rem);
}

int main(int argc, char **argv)
{
	double phi = (1 + sqrt(5)) / 2; // golden mean
	logtime lt(1.0, 30.0, phi);

	do {
		std::cout << "sleeping for " << lt.time() << " seconds..." << std::endl;
		lt.logsleep();
		lt.increment();
	} while (lt.time() < lt.maximum());

	return 0;
}

