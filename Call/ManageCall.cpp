#include "ManageCall.h"
#include "Client/Client.h"
#include "Global/Global.h"
#include "CallAction.h"
#include <QMessageBox>
#include "MainWindow.h"

#undef GetMessage
CManageCall::CManageCall(QObject *parent) :
    QObject(parent)
{
    m_bVideoCall = false;
}

CManageCall::~CManageCall()
{
    LOG_MODEL_DEBUG("CManageCall", "CManageCall::~CManageCall");
}

int CManageCall::Init(QString szId)
{
    bool check = false;
    Q_UNUSED(szId);
    check = connect(GET_CLIENT.data(), SIGNAL(sigCallReceived(QSharedPointer<CCallObject>)),
                    SLOT(slotCallVideoReceived(QSharedPointer<CCallObject>)));
    Q_ASSERT(check);
    return 0;
}

int CManageCall::Clean()
{
    int nRet = 0;
    GET_CLIENT->disconnect(this);
    return nRet;
}

int CManageCall::Call(QString szId, bool bVideo)
{
    m_bVideoCall = bVideo;
    QSharedPointer<CUser> roster = GLOBAL_USER->GetUserInfoRoster(szId);
    if(roster.isNull())
    {
        LOG_MODEL_ERROR("CManageCall", "Don't get roster:%s", qPrintable(szId));
        return -1;
    }
    //检查是否正在视频  
    if(!m_Call.isNull())
    {
        QString szCallId = m_Call->GetId();
        QString szShowName = szCallId;
        QSharedPointer<CUser> callRoster = GLOBAL_USER->GetUserInfoRoster(szCallId);
        if(!callRoster.isNull())
            szShowName = callRoster->GetInfo()->GetShowName();
        roster->GetMessage()->AddMessage(szId, tr("Being talk with %1, please stop it.").arg(szShowName ), true);
        emit GET_CLIENT->sigMessageUpdate(szId);
        return -2;
    }

    QSharedPointer<CCallObject> call = GET_CLIENT->Call(szId, bVideo);
    if(call.isNull())
    {
        LOG_MODEL_DEBUG("CManageCall", "CManageCall::CallVideo fail");
        return -3;
    }
    m_Call = call;
    bool check = connect(call.data(), SIGNAL(sigFinished(CCallObject*)),
                         SLOT(slotCallFinished(CCallObject*)));
    Q_ASSERT(check);
    QSharedPointer<CCallAction> action(new CCallAction(call, szId, QTime::currentTime(), true));
    roster->GetMessage()->AddMessage(action);
    emit GET_CLIENT->sigMessageUpdate(szId);
    return 0;
}

void CManageCall::slotCallVideoReceived(QSharedPointer<CCallObject> call)
{
    QSharedPointer<CUser> roster = GLOBAL_USER->GetUserInfoRoster(call->GetId());
    if(roster.isNull())
    {
        LOG_MODEL_ERROR("CManageCall", "Don't get roster:%s", qPrintable(call->GetId()));
        return;
    }
    //检查是否正在视频  
    if(!m_Call.isNull())
    {
        QString szCallId = m_Call->GetId();
        QString szShowName = szCallId;
        QSharedPointer<CUser> callRoster = GLOBAL_USER->GetUserInfoRoster(szCallId);
        if(!callRoster.isNull())
            szShowName = callRoster->GetInfo()->GetShowName();
        QString szMsg = tr("Has new call from %1. but being talk with %2, please stop it.").arg(roster->GetInfo()->GetShowName(), szShowName);
        roster->GetMessage()->AddMessage(call->GetId(), szMsg, true);
        GET_MAINWINDOW->ShowTrayIconMessage(roster->GetInfo()->GetShowName(), szMsg);
        emit GET_CLIENT->sigMessageUpdate(call->GetId());
        return;
    }

    m_Call = call;
    bool check = connect(m_Call.data(), SIGNAL(sigFinished(CCallObject*)),
                         SLOT(slotCallFinished(CCallObject*)));
    Q_ASSERT(check);

    //监控模式下自动接收  
    if(roster->GetInfo()->GetIsMonitor() && CGlobal::Instance()->GetIsMonitor())
    {
        m_Call->Accept();
    }

    QSharedPointer<CCallAction> action(new CCallAction(m_Call, m_Call->GetId(), QTime::currentTime(), false));

    roster->GetMessage()->AddMessage(action);
    GET_MAINWINDOW->ShowTrayIconMessage(roster->GetInfo()->GetShowName(), 
                                        roster->GetInfo()->GetShowName() + tr(" is calling"));
    emit GET_CLIENT->sigMessageUpdate(call->GetId());
}

void CManageCall::slotCallFinished(CCallObject *pCall)
{
    LOG_MODEL_DEBUG("CManageCall", "CManageCall::slotCallFinished");
    if(m_Call == pCall)
        m_Call.clear();
}

bool CManageCall::IsRun()
{
    if(!m_Call.isNull())
        return true;
    return false;
}

int CManageCall::Stop()
{
    if(!m_Call.isNull())
        m_Call->Cancel();
    return 0;
}

int CManageCall::ProcessCommandCall(const QString &szId, const QString &szCommand)
{
    QString szCmd;
    szCmd = szCommand.split("=").at(1);

    if(m_Call.isNull() && "call" != szCmd)
    {
        LOG_MODEL_ERROR("CManageCall", "m_Call is null or command is error. szCmd:%s", qPrintable(szCmd));
        return -1;
    }
    if("accept" == szCmd)
        m_Call->Accept();
    else if("cancel" == szCmd)
        m_Call->Cancel();
    else if("call" == szCmd)
        Call(szId, m_bVideoCall);
    else
    {
        LOG_MODEL_DEBUG("CManageCall", "command isn't exist.szCmd:%s", qPrintable(szCmd));
        return -1;
    }
    return 0;
}
