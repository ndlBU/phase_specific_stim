#include <default_gui_model.h>
#include <cstdlib> //abs()
#include "../include/RealTimeLogger.h"
#include <QTextStream>
#include <vector>
#include "fftw3.h"

class ptDetector_predict : public DefaultGUIModel
{
	Q_OBJECT	

	public:
		ptDetector_predict(void);
		virtual ~ptDetector_predict(void);
		void execute(void);
		void customizeGUI(void);
		
		enum ext_type_t {
		PEAK =1, TROUGH =-1, TWENTY_HZ = 0, SIX_HZ = 2
		};
	
	protected:
		virtual void update(DefaultGUIModel::update_flags_t);
		
	public slots:	

	signals:

	private:
		void initParameters();
		int getExtremaType(int);
        	int sign(double); // determines sign of sign of number. -1, 0, or 1
        	int peakOrTrough(double); // determines whether data point is at peak or trough
		void fillMean(double);
		void adjustMean(double);
		double findMean();
		double findVariance(double);
		
		ext_type_t extremaType; //Stimulation mode (peak or trough)
		
        	int out;	// output value (1 = stimulation, 0 = no stimulation)
		int type;	// peak or trough detected (peak = 1, trough = -1, nothing = 0)
		double lastpeak; // stores time of most recent peak (local or global)
		double lasttrough; //stores time of most recent trough (local or global)
		double lastglobpeak; // stores time of most recent global peak
		double lastglobtrough; // stores time of most recent global trough
		double laststim; //time of last stimulus (start of refractory period)
		double filtered_LFP; //input voltage

		double Tp; //time of previous data point
		double Tn; //time of current data point (equal to systime)
		double Dp; //derivative of previous data point
		double Dn; //derivative of current data point
       		double Ap; //amplitude of previous data point
		double An; //amplitude of current data point

		double tstart; //start timer
		double futurestimtime; //predicted time to stimulate

		double variance;
		double mean; //value of current running mean
		const int meanbuffersize = 250;
		vector<double> meanbuffer;

		const double variance_threshold = 0;
		
		bool refrac; //stimulate when not in refractory period (stimdur)
		bool search; //true when in searching period for time from median to extrema
		bool peakfound;
		bool troughfound;
        
		//Parameters
		const double delay = 0.06; // s, delay of the filter
		const double quartercycle = 0.02; // s, corresponds to 12Hz
		const double halfcycle_fast = 0.035; // s, corresponds to 13Hz
		const double halfcycle_slow = 0.165; //s, corresponds to 3 Hz
		const double fullcycle_fast = 0.07; // s, corresponds to 12Hz
		const double threshold = 1.5e-4; // in V
		const double stimdur = 0.005; //duration of stimulus in s
		const double T_ERROR = 0.0015; //Accounts for difference between current time and desired stimtime

		double dt;	// system period
		double systime;	// runtime of command
		long long count;	// counter for samples executed during current command

		//QT components
		QComboBox *extremaTypeSelect;
		
		// DataLogger
		RealTimeLogger *data;
		double maxt, tcnt;
		int acquire, cols;
		int cellnum, tempcell;
		string prefix, info, path;
		double pdone;
		double raw_LFP;

	private slots:
		void updateType(int);
};
