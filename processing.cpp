﻿//---------------------------------------------------------------------------

#pragma hdrstop

#include "processing.h"
#include "languages.h"
#include "System.hpp"
#include "System.NetEncoding.hpp"
#include <System.JSON.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)


void processBlank(Message message)
{
	addLog(TYPE_LOG_ERROR, "Обработчик отсутствует");
}

void processInfo(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришли реквизиты");

	if (message.messages[0].Length() && message.messages[1].Length()) {
		myClient.pid = message.messages[0];
		myClient.pass = message.messages[1];
	}
	myClient.version = message.messages[2];
	myClient.webpanel = message.messages[3];
	myClient.webprofile = message.messages[4];

	myClient.profilelogin = message.messages[5];
	myClient.profilepass = message.messages[6];

	if (myClient.profilelogin.Length() && myClient.profilepass.Length() && myClient.pid.Length()) {
        PostMessage(getMainHandle(), WM_VISIT_ALOGIN, 0, 0);
	}
}

void processNotification(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришло уведомление");

	//todo ну тут есть нюансы, мы не можем все подряд отображать уведомления, к тому же надо избавиться от уведомлений которые генерирует сервер на русском языке
//	UnicodeString *buf = new UnicodeString(message.messages[0]);
//	PostMessage(getMainHandle(), WM_VISIT_NOTIF, 0, (long)buf);
}

void processInfoClient(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришла информация о VNC");

	myClient.client = message.messages[0];
	myClient.manage = message.messages[1];
}

void processTerminate(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришёл запрос на завершение");

	PostMessage(getMainHandle(), WM_VISIT_EXIT, 0, 0);
}

void processLogin(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришёл отзыв на логин");

	PostMessage(getMainHandle(), WM_VISIT_LOGIN, 0, 0);
}

void processContact(Message message)
{
//	addLog(TYPE_LOG_INFO, "Пришёл контакт");

	if (message.messages[1] == "del") {
		Contact *c = getContact(myClient.contact, StrToInt(message.messages[0]));
		if (c != NULL) {
			PostMessage(getMainHandle(), WM_VISIT_DCONT, 0, (long)c);
		}
	}
	else
	{
		Contact *n = getContact(myClient.contact, StrToInt(message.messages[0]));
		if (n != NULL) {
			n->caption = message.messages[2];
			n->pid = message.messages[3];
			PostMessage(getMainHandle(), WM_VISIT_CONT, 0, (long)n);

			if (message.messages[5].Length()) { //если есть это поле, значит переместили контакт
			
				myClient.contact = delContact(myClient.contact, n->id);
			
				int id = StrToInt(message.messages[5]);
				if (id == -1) {
					n->next = myClient.contact;
					myClient.contact = n;		
				}
				else {
					Contact *c = getContact(myClient.contact, id);
	
					if (c != NULL) { //нашелся новый родитель
						n->next = c->inner;
						c->inner = n;
					}
					else { //не нашли куда его девать
						delete n;
						return;
					}
				}				

				PostMessage(getMainHandle(), WM_VISIT_MCONT, id + 1, (long)n);
			}
		}
		else { //новый контакт
			Contact *n = new Contact;
			n->id = StrToInt(message.messages[0]);
			n->type = message.messages[1];
			n->caption = message.messages[2];
			n->pid = message.messages[3];
			n->pas = message.messages[4];

			n->data = NULL;
			n->next = NULL;
			n->inner = NULL;

			int id = StrToInt(message.messages[5]);
			if (id == -1) { //родительская папка
				n->next = myClient.contact;
				myClient.contact = n;
			}
			else {
				Contact *c = getContact(myClient.contact, id);
				if (c != NULL) { //нашелся родитель
					n->next = c->next;
					c->next = n;
				}
				else { //не нашли куда его девать
					delete n;
					return;
				}
			}

			PostMessage(getMainHandle(), WM_VISIT_CONT, id + 1, (long)n);
		}
	}
}

void processContacts(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришёл список контактов");

	TURLEncoding *coder = new TURLEncoding;
	UnicodeString contacts = coder->URLDecode(message.messages[0]);
	coder->Free();

	myClient.contact = parseContact(contacts);

	PostMessage(getMainHandle(), WM_VISIT_UPDATE, 0, 0);
}

void processLogout(Message message)
{
	addLog(TYPE_LOG_INFO, "Мы вышли из профиля");

	PostMessage(getMainHandle(), WM_VISIT_DISCON, 0, 0);
}

void processExec(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришла команда на запуск программы");

	UnicodeString *buf = new UnicodeString(message.messages[0]);

	PostMessage(getMainHandle(), WM_VISIT_EXEC, 0, (long)buf);
}

void processStatus(Message message)
{
//	addLog(TYPE_LOG_INFO, "Пришел статус контакта");

	Contact *c = getContact(myClient.contact, cleanPid(message.messages[0]));
	if(c != NULL){
		c->status = StrToInt(message.messages[1]);
		PostMessage(getMainHandle(), WM_VISIT_SCONT, 0, (long)c);
	}
}

void processListVNC(Message message)
{
//	addLog(TYPE_LOG_INFO, "Пришла позиция vnc");

	UnicodeString *buf = new UnicodeString(message.messages[0].c_str());
	PostMessage(getMainHandle(), WM_VISIT_IVNC, 0, (long)buf);
}

void processInfoAnswer(Message message)
{
//	addLog(TYPE_LOG_INFO, "Пришла информация о контакте");

	UnicodeString **buf = new UnicodeString*[3];

	buf[0] = new UnicodeString(message.messages[2].c_str());
	buf[1] = new UnicodeString(message.messages[3].c_str());
	if (message.count_poles > 3) {
		buf[2] = new UnicodeString(message.messages[4].c_str());
	} else {
		buf[2] = new UnicodeString("");
	}

	PostMessage(getMainHandle(), WM_VISIT_INCLNT, WPARAM(buf), StrToInt(message.messages[1]));
}

void processReload(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришла запрос на перезапуск UI");

	if(ExistService()) {
		PostMessage(getMainHandle(), WM_VISIT_SRLOAD, 0, 0);
	}
	else {
		PostMessage(getMainHandle(), WM_VISIT_RELOAD, 0, 0);
	}
}

void processLog(Message message)
{
	addLog(TYPE_LOG_INFO, message.messages[0]);
}

void processHide(Message message)
{
	 myClient.hide = message.messages[0];
}

void processOptionsUI(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришли опции для UI");

	TURLEncoding *coder = new TURLEncoding;
	UnicodeString options = coder->URLDecode(message.messages[0]);
	coder->Free();

	TJSONObject *json = (TJSONObject*) TJSONObject::ParseJSONValue(options);

	TJSONPair *pair = json->Get("Width");
	if(pair) myOptions.Width = StrToInt(pair->JsonValue->Value());

    pair = json->Get("Width");
	if(pair) myOptions.Width = StrToInt(pair->JsonValue->Value());

	pair = json->Get("Height");
	if(pair) myOptions.Height = StrToInt(pair->JsonValue->Value());

	pair = json->Get("Left");
	if(pair) myOptions.Left = StrToInt(pair->JsonValue->Value());

	pair = json->Get("Top");
	if(pair) myOptions.Top = StrToInt(pair->JsonValue->Value());

	pair = json->Get("TrayIcon");
	if(pair) myOptions.TrayIcon = StrToInt(pair->JsonValue->Value());

	pair = json->Get("Lang");
	if(pair)
		myOptions.Lang = StrToInt(pair->JsonValue->Value());
	else
        myOptions.Lang = 0;

	json->Free();

	PostMessage(getMainHandle(), WM_VISIT_APPLY, 0, 0);
}

void processProxy(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришли настройки прокси");

	UnicodeString **buf = new UnicodeString*[2];

	buf[0] = new UnicodeString(message.messages[0].c_str());
	buf[1] = new UnicodeString(message.messages[1].c_str());

	PostMessage(getMainHandle(), WM_VISIT_PROXY, WPARAM(buf), 0);
}

void processStandartAlert(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришло стандартное сообщение");

	UnicodeString *buf;

	switch (StrToInt(message.messages[0])) {
		case 1:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_NETWORK_MESSAGE) + ".");
			break;
		case 2:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_PROXY_MESSAGE) + ".");
			break;
		case 3:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_AUTH_MESSAGE) + ".");
			break;
		case 4:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_VNC_MESSAGE) + ".");
			break;
		case 5:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_TIMEOUT_MESSAGE) + ".");
			break;
		case 6:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_PEER_MESSAGE) + ".");
			break;
		case 7:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_TYPE_MESSAGE) + ".");
			break;
		default:
			buf = new UnicodeString(getLangText(L_ERROR_MESSAGE) + getLangText(L_ALERT_EMPTY_MESSAGE) + ".");
			break;
	}

	PostMessage(getMainHandle(), WM_VISIT_NOTIF, 0, (long)buf);
}

