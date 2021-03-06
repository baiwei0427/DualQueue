/*
 * Multi-Level Feedback Queue (MLFQ)
 * Copyright (c) 2014 Wei Bai (baiwei0427@gmail.com) from Hong Kong University of Science and Technology
 * All rights reserved.
 *
 * In MLFQ, it contains N CoS (Class of Service) queues.
 * MLFQ uses Strict Priority (SP) to schedule packets among multiple CoS queues.
 * To cooperate with ECN, MLFQ also has a ECN marking threshold.
 * When the queue lengths of all CoS queues exceed this threshold, ECN is marked.
 * You can select enqueue marking or dequeue marking
 *
 * Variables:
 * queue_num_: number of CoS queues 
 * thresh_: ECN marking threshold
 * dequeue_marking_: 1 (dequeue marking) or 0 (enqueue_marking) 
 * mean_pktsize_: configured mean packet size in bytes
 */
 
#include "mlfq.h"
#include "flags.h"

static class MLFQClass : public TclClass {
 public:
	MLFQClass() : TclClass("Queue/MLFQ") {}
	TclObject* create(int, const char*const*) {
		return (new MLFQ);
	}
}class_mlfq;

void MLFQ::enque(Packet* p)
{
	hdr_ip *iph = hdr_ip::access(p);
	int prio = iph->prio();
	hdr_flags* hf = hdr_flags::access(p);

	int qlimBytes = qlim_ * mean_pktsize_;
	
	//queue length exceeds the queue limit
	if(queueByteLength(queue_num_)+hdr_cmn::access(p)->size()>qlimBytes)
	{
		drop(p);
		return;
	}
	
	if(prio>=queue_num_)
		prio=queue_num_-1;
		
	//ECN enqueue marking and normal marking
	if(dequeue_marking_==0 && prio_marking_==0)
	{
		if(queueByteLength(queue_num_)+hdr_cmn::access(p)->size()>thresh_*mean_pktsize_)
		{
			if (hf->ect()) //If this packet is ECN-capable 
				hf->ce() = 1;
		}
	}
	//ECN enqueue marking and priority marking
	else if(dequeue_marking_==0 && prio_marking_==1)
	{
		if(queueByteLength(prio+1)+hdr_cmn::access(p)->size()>thresh_*mean_pktsize_)
		{
			if (hf->ect()) //If this packet is ECN-capable 
				hf->ce() = 1;
		}
	}
	
	//Enqueue packet
	q_[prio]->enque(p);
}

Packet* MLFQ::deque()
{
	//high->low: 0->7
	for(int i=0;i<queue_num_;i++)
	{
		if(q_[i]->length()>0)
		{
			Packet* p=q_[i]->deque();
			//ECN dequeue marking and normal marking 
			if(dequeue_marking_==1&& prio_marking_==0)
			{
				if(queueByteLength(queue_num_)>thresh_*mean_pktsize_)
				{
					hdr_flags* hf = hdr_flags::access(p);
					if (hf->ect()) //If this packet is ECN-capable 
						hf->ce() = 1;
				}
			}
			//ECN dequeue marking and priority marking 
			else if(dequeue_marking_==1&& prio_marking_==1)
			{
				if(queueByteLength(i+1)>thresh_*mean_pktsize_)
				{
					hdr_flags* hf = hdr_flags::access(p);
					if (hf->ect()) //If this packet is ECN-capable 
						hf->ce() = 1;
				}
			}
			
			//dequeue packet
			return (p);
		}
	}
	return q_[0]->deque();
}

