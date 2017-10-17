//
// Created by zzyyy on 2017/4/15.
//

#ifndef SSHKV_STATUS_H
#define SSHKV_STATUS_H

namespace sshkv{

	enum STATUS{ERROR = -1, FAILED, SUCCESS, FULL, NOTFOUND};

	enum BLOCK_STATE{FREE, DIRTY, VALID};

}//namespace sshkv
#endif //SSHKV_STATUS_H
