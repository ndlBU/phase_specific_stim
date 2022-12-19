#include <cstdlib> //abs()
#include <default_gui_model.h>
#include <main_window.h>
#include <string>
#include <vector>
#include <QTextStream>
#include "ptDetector_predict.h"
#include <iostream>
#include "fftw3.h"
#include <cmath>

extern "C" Plugin::Object *createRTXIPlugin(void)
{
	return new ptDetector_predict();
}

static DefaultGUIModel::variable_t vars[] =
{
	{ "Vin", "Local field potential (V)", DefaultGUIModel::INPUT, },
	{ "RawVin", "Unfiltered Voltage of LFP (V)", DefaultGUIModel::INPUT, },
	{ "Stimulation", "Output simulation (0 = no stimulation, 1 = stimulation", DefaultGUIModel::OUTPUT, },
	{ "Variance", "Variance", DefaultGUIModel::OUTPUT, },
	{ "Detrended Vin", "Detrended filtered LFP voltage", DefaultGUIModel::OUTPUT, },
	{ "Extrema Found", "Trough identified", DefaultGUIModel::OUTPUT, },
	{ "Acquire?", " 0 or 1", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{ "Cell Number", "Cell identifier",
		DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{ "Record Length (s)", "duration of recording session",
		DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{ "File Path", "", DefaultGUIModel::COMMENT,},
	{ "File Prefix", "", DefaultGUIModel::COMMENT,},
	{ "File Info", "", DefaultGUIModel::COMMENT,},
	{ "Time (s)", "Time (s)", DefaultGUIModel::STATE, }, 
	{ "Percent Done", "", DefaultGUIModel::STATE | DefaultGUIModel::DOUBLE, }	  
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);
 
ptDetector_predict::ptDetector_predict(void) : DefaultGUIModel("PeakTroughPredictiveDetect", ::vars, ::num_vars)
{
	setWhatsThis(
		"<p><b>Peak and Trough Detector:</b><br>This module detects and stimulates peaks or troughs of theta waves with a predictive approach.</p>");
	initParameters();
	DefaultGUIModel::createGUI(vars, num_vars);
	customizeGUI();
	update(INIT);
	refresh();
	resizeMe();
}

ptDetector_predict::~ptDetector_predict(void)
{
}

void ptDetector_predict::execute(void) 
{
	systime = count * dt; // time in seconds
	Tn = systime; // Tn is current time in seconds

	An = input(0); // current amplitude
	raw_LFP = input(1); // voltage of unfiltered LFP

	out = 0;
	int ext_found = 0;	
	
	if (extremaType == TWENTY_HZ)
	{
		if (count % 50 == 0)
		{
			output(0) = 5;
			laststim = Tn;
		}
		else if ((Tn - laststim) <= stimdur)
		{
			output(0) = 5;
		}
		else
		{
			output(0) = 0;
		}
	}
	else if (extremaType == SIX_HZ)
	{
		if (count % 165 == 0)
		{
			output(0) = 5;
			laststim = Tn;
		}
		else if ((Tn - laststim) <= stimdur)
		{
			output(0) = 5;
		}
		else
		{
			output(0) = 0;
		}
	}
	else 
	{
	if (count < meanbuffersize)
	{
		fillMean(An);
	}
	else // enter else statement once buffer is filled
	{
		//Store new value in mean buffer and find mean
		adjustMean(An);
		mean = findMean();		
		variance = findVariance(mean);	

		//Calculate new derivative		
		Dn = (An-Ap)/(Tn-Tp);

		//In refractory period
		if ((Tn - laststim) <= stimdur)	
		{
			refrac = true;
		}
		else
		{
			refrac = false;
		}

		//Peak stimulation
		if (extremaType == 1) 
		{
			//Found extrema
			if (sign(Dn) != sign(Dp))
			{
				type = peakOrTrough(Dn);
				//Found a peak				
				if (type == 1)
				{
					//New global peak
					if (An > lastpeak)
					{
						tstart = Tn;
						lastglobpeak = Tn;
						lastpeak = An;
						peakfound = true;
						ext_found = 1;
					}
				}
				//Found a trough
				else if (type == -1 && peakfound)
				{
					
					if ((Tn - lastglobpeak) > halfcycle_slow)
					{
						lastpeak = -99;
						peakfound = false;
					}	
					else if ((Tn - lastglobpeak) >= halfcycle_fast)
					{					
						futurestimtime = Tn + (Tn - tstart) - delay;
						lastglobtrough = Tn;
						lasttrough = An;
						peakfound = false;
						ext_found = -1;
						lastpeak = -99;
					}
					
				}
			}

		}
		//Trough stimulation
		else if (extremaType == -1) 
		{
			//Found extrema
			if (sign(Dn) != sign(Dp))
			{
				type = peakOrTrough(Dn);
				//Found a trough				
				if (type == -1)
				{
					//New global trough
					if (An < lasttrough)
					{
						tstart = Tn;
						lastglobtrough = Tn;
						lasttrough = An;
						troughfound = true;
						ext_found = -1;
					}
				}
				//Found a peak
				else if (type == 1 && troughfound)
				{
					if ((Tn - lastglobtrough) > halfcycle_slow)
					{
						lasttrough = 99;
						troughfound = false;
					}
					//New globalpeak
					else if ((Tn - lastglobtrough) >= halfcycle_fast)
					{					
						futurestimtime = Tn + (Tn - tstart) - delay;
						lastglobpeak = Tn;
						lastpeak = An;
						troughfound = false;
						ext_found = 1;
						lasttrough = 99;
					}
				}
			}

		}

		//Set all previous time, amplitude, and derivative values to current values
		Tp = Tn;
		Ap = An;
		Dp = Dn;

		//Set output
		if ((!refrac) && (Tn - laststim >= fullcycle_fast) && (variance >= variance_threshold))
		{
			if (abs(futurestimtime - Tn) <= T_ERROR)
			{
				out = 5; // sets out to 1 to stimulate
				laststim = Tn; // Remember time of the beginning of the last stimulation
				futurestimtime = 0;
			}
			else if ((futurestimtime - Tn <= 0) && (futurestimtime - Tn >= -0.018))
			{
				out = 5; // sets out to 1 to stimulate
				laststim = Tn; // Remember time of the beginning of the last stimulation
				futurestimtime = 0;
			}
		}

		//If REFRAC is true, OUT continues to be 1 and stimulation occurs
		if (refrac)
		{
			out = 5;
		}
	}
		
	output(0) = out;
	output(1) = variance;	
	output(2) = An;
	output(3) = ext_found;
	
	}

	count++;
	
	//Record data if acquire is 1
	if (acquire && tcnt < maxt) 
	{
		data->insertdata(tcnt);
		data->insertdata(raw_LFP);
		data->insertdata(An);
		data->insertdata(out);
		data->insertdata(extremaType);
		
		tcnt += dt;
		pdone = (tcnt/maxt)*100;
	
	} 
	else if (acquire && tcnt > maxt) 
	{
		tcnt = 0;		// reset timer
		acquire = 0;	// reset acquire signal
		
		data->writebuffer(prefix,info);
		data->resetbuffer();
	}
}


int ptDetector_predict::sign(double x)
{
    if (x > 0)
        return 1; //positive
    else if (x < 0)
        return -1; //negative
    else
        return 0; //zero
}

int ptDetector_predict::peakOrTrough(double d)
{
    //Assumes that current derivative and previous derivative are different signs already
    //Peak = 1, Trough = -1
    if (sign(d) == 1)
        return -1; //trough
    else if (sign(d) == -1)
        return 1; //peak
    else
        return 0;
}

void ptDetector_predict::fillMean(double newval)
{
	meanbuffer.push_back(newval); //adds newval to end and size increases by one 
}

void ptDetector_predict::adjustMean(double newval)
{
	meanbuffer.erase(meanbuffer.begin()); //erases element at the beginning and size decreases by one
	meanbuffer.push_back(newval); //adds newval to end and size increases by one 
}

double ptDetector_predict::findMean()
{
	double Cm = std::accumulate(meanbuffer.begin(),meanbuffer.end(),0.0)/(meanbuffersize); 
	return Cm;
}

double ptDetector_predict::findVariance(double m)
{
	std::vector<double>::iterator it;	
	double v = 0;
	for(it = meanbuffer.begin(); it != meanbuffer.end(); it++)
	{
		v += (*it-m)*(*it-m);
	}
	return v/meanbuffersize;
}


int ptDetector_predict::getExtremaType(int index)
{
	switch(index) {
		case 0:
			return PEAK;
		case 1:
			return TROUGH;
		case 2:
			return TWENTY_HZ;
	case 3:
	  return SIX_HZ;
	}
}

void ptDetector_predict::updateType(int index)
{
	switch(index)
	{
		case 0:
			extremaType = PEAK;
			break;
		case 1:
			extremaType = TROUGH;
			break;
		case 2:
			extremaType = TWENTY_HZ;
			break;
	case 3:
			extremaType = SIX_HZ;
			break;
	}
}

void ptDetector_predict::update(DefaultGUIModel::update_flags_t flag)
{
	switch (flag) {
	case INIT:
	qDebug("INIT is called");
		setState("Time (s)", systime);
		extremaTypeSelect->setCurrentIndex(0);

		setParameter("Record Length (s)", QString::number(maxt));	
		setParameter("Acquire?",acquire);
		setParameter("Cell Number",cellnum);
		setComment("File Path", QString::fromStdString(path));
		setComment("File Prefix", QString::fromStdString(prefix));
		setComment("File Info", QString::fromStdString(info));
		setState("Percent Done", pdone);
		
		break;
		
	case MODIFY:
	qDebug("MODIFY is called");
		extremaType = ext_type_t(getExtremaType(extremaTypeSelect->currentIndex()));

		acquire = getParameter("Acquire?").toUInt();
		tempcell = getParameter("Cell Number").toInt();

	qDebug()<< QString::fromStdString(path);
	qDebug()<< getComment("File Path");

		path = getComment("File Path").toStdString();
		prefix = getComment("File Prefix").toStdString();
		info = getComment("File Info").toStdString();
		maxt = getParameter("Record Length (s)").toDouble();		
	qDebug()<< QString::fromStdString(path);

		data->deleteBuffer();
		data->resetbuffer();
		data->setBufferLen((int)(cols*maxt/dt));
		if (tempcell != cellnum) {
			data->newcell(tempcell);
			cellnum = tempcell;
		}		

		// reset the timers
		pdone = 0.0;
		tcnt = 0.0;
		systime = 0;
		count = 0;
		Tp = 0;
		Tn = 0;
		Dp = 0;
		Dn = 0;
		Ap = 0;
		An = 0;
		variance = 0;
		mean = 0;
		futurestimtime = 0;
		meanbuffer.clear();
		type = 0;
		tstart = 0;
		lastpeak = -99; 
		lasttrough = 99;
		lastglobpeak = 0;
		lastglobtrough = 0;
		laststim = 0;
		refrac = false;
		search = false;
		peakfound = false;
		troughfound = false;
	
		//reset output
		out = 0;
		break;
		
	case PERIOD:
		dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
		break;
	case PAUSE:
		output(0) = 0.0;
		variance = 0;
		mean = 0;
		break;
	case UNPAUSE:
		systime = 0;
		count = 0;
		Tp = 0;
		Tn = 0;
		Dp = 0;
		Dn = 0;
		Ap = 0;
		An = 0;
		variance = 0;
		mean = 0;
		futurestimtime = 0;
		meanbuffer.clear();
		type = 0;
		tstart = 0;
		lastpeak = -99; 
		lasttrough = 99;
		lastglobpeak = 0;
		lastglobtrough = 0;
		laststim = 0;
		refrac = false;
		search = false;
		peakfound = false;
		troughfound = false;
		break;
	default:
		break;
	}
}

void ptDetector_predict::initParameters()
{
	extremaType = PEAK;
	output(0) = 0;
	out = 0;
	dt = RT::System::getInstance()->getPeriod() * 1e-9; // s
	systime = 0;
	count = 0;
	
	type = 0;
    	lastpeak = -99; 
    	lasttrough = 99; 
	lastglobpeak = 0;
	lastglobtrough = 0;
	laststim = 0;
	refrac = false;
	search = false;
	peakfound = false;
	troughfound = false;

	meanbuffer.reserve(meanbuffersize);

	Tp = 0;
	Tn = 0;
	Dp = 0;
	Dn = 0;
  	Ap = 0;
	An = 0;
	mean = 0;
	variance = 0;
	futurestimtime = 0;
	
	cellnum = 1;
	acquire = 0;
	cols = 8;
	
	tcnt = 0.0;
	maxt = 0.0;
	path = "/home/ndl/RTXI/data/";
	prefix = "Theta_Stimulation";
	info = "1kHz: sys_time, raw_LFP, V, Stim, Mode";

	qDebug()<< QString::fromStdString(path);
	
	data  = new RealTimeLogger((int)(cols*maxt/dt), cols, path);
	data->newcell(cellnum);
	data->setDSRate(1);
	data->setPrint(1);
	
	pdone = 0.0;

}

void ptDetector_predict::customizeGUI(void)
{
	QGridLayout *customlayout = DefaultGUIModel::getLayout();

	QGroupBox *formGroupBox = new QGroupBox("Stimulation Mode");
	QFormLayout *layout = new QFormLayout(formGroupBox);
	
	extremaTypeSelect = new QComboBox;
	extremaTypeSelect->insertItem(0,"Peak");
	extremaTypeSelect->insertItem(1,"Trough");
	extremaTypeSelect->insertItem(2,"20 Hz");
	extremaTypeSelect->insertItem(3,"6 Hz");
	QObject::connect(extremaTypeSelect,SIGNAL(activated(int)), this, SLOT(updateType(int)));

	layout->addRow(tr("Stimulation Mode:"), extremaTypeSelect);
	
	formGroupBox->setLayout(layout);

	customlayout->addWidget(formGroupBox,0,0);
	setLayout(customlayout);
}
