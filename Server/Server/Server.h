/*
 * Server.h
 *
 *  Created on: 2017/01/08
 *      Author: hira
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "PbvrFilterInformation.h"
#include "Argument.h"
#include "FilterInformation.h"
#include "ParticleTransferProtocol.h"

class Server {
public:
    Server();
    Server(const Argument *param);
    virtual ~Server();
    FilterInformationList* loadFilterInformationList();
    bool isLoadPbvrFilter();
    std::string &getLoadFilterFile();
    int send_bcast(const jpv::ParticleTransferClientMessage *mes);
    int recv_bcast(jpv::ParticleTransferClientMessage *mes);
private:
    const Argument *m_param;
    bool m_isloadpbvrfilter;
    std::string m_loadfilename;

};


#endif /* SERVER_H_ */
