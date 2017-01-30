/*
*                         Vortex OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2017 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/

#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

#include "VortexGrove_DCPS.hpp"

/* C API for GrovePi Module */
#include "grovepi.h"

using namespace std;

int main (int argc, char *argv[])
{
    /** A dds::domain::DomainParticipant is created for the default domain. */
    dds::domain::DomainParticipant participant(org::opensplice::domain::default_id());
	cout << "=== [LEDSubscriber] Participant Created" << endl;
	
	/* A dds::topic::qos::TopicQos is created with Reliability set to Reliable to
	* guarantee delivery. */

    dds::topic::qos::TopicQos reliableTopicQos = participant.default_topic_qos() 
		<< dds::core::policy::Reliability::Reliable();

    /** Set the Reliable TopicQos as the new default */
    participant.default_topic_qos(reliableTopicQos);

	/* A dds::topic::Topic is created for the Analogue Sensor types on the
	* domain participant. 
	* The full topic name will be "VortexGrove_LED" */

	dds::topic::Topic<VortexGrove::LED> LEDTopic(participant,
		"VortexGrove_LED", reliableTopicQos);
	cout << "=== [LEDSubscriber] Topic Created " << endl;

	/* Create a Subscriber and Data Reader.
	* QoS Settings comply with that on the topic.
	* All default QoS values for an Entity can be found in the language reference guide. */

	dds::sub::qos::SubscriberQos subQos = participant.default_subscriber_qos();
	
	dds::sub::Subscriber LEDSubscriber(participant, subQos);

	dds::sub::qos::DataReaderQos drQos = LEDTopic.qos();

	dds::sub::DataReader<VortexGrove::LED> LEDDR(LEDSubscriber, LEDTopic, drQos);

	/* Create a unique reference for the Temperature and Lux values.
	* This reference is also used as the Topic Key */
	short RotaryID = 100;
	short LuxID = 200;

	/* Initialise Grove Pi Board */

	if (init() == -1)
	{
		cout << "=== [LEDSubscriber] GrovePi Failed to initialise" << endl;
		cout << "=== [LEDSubscriber] Check the GrovePi is online (green LED on top)\n"
			 << "=== [LEDSubscriber] Test the I2C port connection with command: \"i2cdetect -y 1\"" << endl;
		return 0;
	}
	else 
	{
		cout << "=== [LEDSubscriber] Grove Pi initialised" << endl;
	}


	int count = 0;

	/* Readers created, we need to Loan some memory to store received samples
	 * the memory is returned once the Loan runs out of scope */
	cout << "=== [LEDSubscriber] Serving Light Sensor LED on D4..." << endl;
	cout << "=== [LEDSubscriber] Serving Rotary Sensor LED on D3..." << endl;

	while (1) 
	{
		/* GrovePi.h function, 0 turns off the LED */
		digitalWrite(4, 0);
		digitalWrite(3, 0);


		/* Readers created we need to Loan some memory to store received samples
		* the memory is returned once the Loan runs out of scope */
		dds::sub::LoanedSamples<VortexGrove::LED> Samples = LEDDR.take();

		for (dds::sub::LoanedSamples<VortexGrove::LED>::const_iterator sample = Samples.begin();
			sample < Samples.end(); sample++)
		{ 
			/* Check the sample we received is valid */
			if (sample->info().valid())
			{
				/* Should the LED be on? Which sensor did the sample come from? */
				if (sample->data().on() == true)
				{
					if(sample->data().id() == LuxID)
						digitalWrite(4, 1);

					if (sample->data().id() == RotaryID)
						digitalWrite(3, 1);
				
				}
			}
		} 
		sleep(1);
		count++;
	}

    return 0;
}
