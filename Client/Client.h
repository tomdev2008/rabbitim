#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include "../UserInfo/User.h"
#include "FileTransfer/FileTransfer.h"
#include "Call/CallObject.h"

/**
 * @brief 通信客户端抽象类  
 *
 */
class CClient : public QObject
{
    Q_OBJECT
public:
    explicit CClient(QObject *parent = 0);
    virtual ~CClient();

    virtual bool IsLogin();///< 用户是否登录  

    /**
     * @brief 注册新用户  
     *
     * @param szId:用户id  
     * @param szName:用户名  
     * @param szPassword:密码  
     * @param szEmail:email  
     * @param szDescript:描述  
     * @return int  
     */
    virtual int Register(const QString &szId,
                         const QString &szName,
                         const QString &szPassword, 
                         const QString& szEmail = QString(),
                         const QString& szDescript = QString()) = 0;
    virtual int Register(QSharedPointer<CUserInfo> userInfo, const QString &szPassword) = 0;

    /**
     * @brief 登录  
     *
     * @param szUserName：用户名  
     * @param szPassword：登录密码  
     * @param status：登录状态  
     * @return int：成功返回0，失败返回非0  
     */
    virtual int Login(const QString& szUserName = QString(),
                      const QString &szPassword = QString(),
                      CUserInfo::USER_INFO_STATUS status = CUserInfo::Online) = 0;
    virtual int Logout() = 0;
    /**
     * @brief 请求本地用户信息  
     *
     * @return int：成功返回0，失败返回非0  
     */
    virtual int RequestUserInfoLocale();
    /**
     * @brief 请求指定好友的信息  
     *
     * @param szId：好友ID
     * @return int：成功返回0，失败返回非0  
     */
    virtual int RequestUserInfoRoster(const QString& szId);
    /**
     * @brief 设置用户状态· 
     *
     * @param status：状态  
     * @return int：成功返回0，失败返回非0  
     */
    virtual int setClientStatus(CUserInfo::USER_INFO_STATUS status);

    /**
     * A:请求订阅B  
     * B:接收 A 订阅请求信号  
     *   同意：  
     *   不同意：  
     */
    enum SUBSCRIBE_TYPE
    {
        SUBSCRIBE_REQUEST,
        SUBSCRIBE_ACCEPT,
        SUBSCRIBE_REFUSE
    };

    /**
     * @brief //增加好友  
     *
     * @param szId：好友id
     * @return int：成功返回0，失败返回非0；-1:好友已经存在  
     */
    virtual int RosterAdd(const QString& szId, SUBSCRIBE_TYPE type = SUBSCRIBE_REQUEST,
                          const QString &szName = QString(), const QSet<QString> &groups = QSet<QString>());
    /**
     * @brief 删除好友  
     *
     * @param szId：好友Id
     * @return int：成功返回0，失败返回非0  
     */
    virtual int RosterRemove(const QString& szId);
    /**
     * @brief 重命名好友名称  
     *
     * @param szId:好友id  
     * @param szName:好友名称  
     * @return int：成功返回0，失败返回非0  
     */
    virtual int RosterRename(const QString& szId, const QString& szName);

    /**
     * @brief 向好友发送消息  
     *
     * @param szId:好友id  
     * @param szMsg:消息内容  
     * @return int  
     */
    virtual int SendMessage(const QString& szId, const QString &szMsg);
    /**
     * @brief 设置登录用户信息  
     *
     * @param userInfo
     * @return int：成功返回0，失败返回非0  
     */
    virtual int setlocaleUserInfo(QSharedPointer<CUserInfo> userInfo);

    /**
     * @brief 传送文件  
     *
     * @param szId：用户id  
     * @param szFile：要传送的文件 
     * @param szDescription：描述信息  
     * @return QSharedPointer<CFileTransfer>：返回 CFileTransfer 对象指针  
     */
    virtual QSharedPointer<CFileTransfer> SendFile(const QString szId, const QString &szFile, const QString &szDescription);

    /**
     * @brief 视频呼叫  
     *
     * @param szId：用户id  
     * @return QSharedPointer<CCallObject>
     */
    virtual QSharedPointer<CCallObject> CallVideo(const QString szId);
    /**
     * @brief 语音呼叫  
     *
     * @param szId：用户id  
     * @param bVideo:是否是视频呼叫  
     * @return QSharedPointer<CCallObject>
     */
    virtual QSharedPointer<CCallObject> Call(const QString szId, bool bVideo = false);
    enum ERROR_TYPE
    {
        NoError,                    ///< No error.
        NetworkError,        ///< Error due to TCP socket.
        KeepAliveError,     ///< Error due to no response to a keep alive.
        LoginFail,                 ///< 登录失败  
        NotAuthorized,     ///<验证错误  
        Conflict,                   ///< 用户已经注册  
        OtherError,             ///< 其它错误  
        InternalServerError
    };

signals:
    /**
     * @brief 用户成功登录后触发，界面可以初始化  
     *
     */
    void sigClientConnected();
    /**
     * @brief 用户登出后触发，通知界面析构  
     *
     */
    void sigClientDisconnected();
    void sigClientError(CClient::ERROR_TYPE e);

    /**
     * @brief 从文件中加载好友完成后触发，通知界面初始化数据  
     *
     */
    void sigLoadRosterFromStorage();
    /**
     * @brief 更新好友信息.新增好友会触发此消息.好友列表显示控件，根据好友列表中是否有此好友来决定是否插入或更新好友  
     *
     * @param szId：更新好友信息  
     */
    void sigUpdateRosterUserInfo(const QString &szId, QSharedPointer<CUser> user);
    /**
     * @brief 删除好友时触发  
     *
     * @param szId：好友ID  
     */
    void sigRemoveRosterUserInfo(const QString &szId);
    /**
     * @brief 更新本地用户信息  
     *
     */
    void sigUpdateLocaleUserInfo();
    /**
     * @brief 好友状态改变时触发  
     *
     * @param szId:好友ID  
     */
    void sigChangedStatus(const QString& szId);

    /**
     * @brief 接收增加好友请求时触发  
     *
     * @param szId：请求者ID  
     */
    void sigRosterAddReceived(const QString& szId, const CClient::SUBSCRIBE_TYPE &type);
    /**
     * @brief 删除好友请求时触发  
     *
     * @param szId：删除好友的ID  
     */
    void sigRosterRemoveReceived(const QString& szId);

    /**
     * @brief 接收到新消息时触发  
     *
     * @param szId:好友ID  
     */
    void sigMessageUpdate(const QString& szId);

    /**
     * @brief 清空未读消息时触发  
     *
     * @param szId:好友ID  
     */
    void sigMessageClean(const QString& szId);

    /**
     * @brief 文件接收时触发  
     *
     * @param szId:用户ID  
     * @param file
     */
    void sigFileReceived(const QString &szId, QSharedPointer<CFileTransfer> file);

    /**
     * @brief 视频接收时触发  
     *
     * @param call
     */
    void sigCallReceived(QSharedPointer<CCallObject> call);

public slots:

protected:
    int SetLogin(bool bLogin);
private:
    bool m_bIsLogin;
};

#endif // CLIENT_H
