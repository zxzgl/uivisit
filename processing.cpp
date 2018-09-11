﻿//---------------------------------------------------------------------------

#pragma hdrstop

#include "processing.h"
#include "System.hpp"
#include "System.NetEncoding.hpp"
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

	myClient.profilelogin = message.messages[4];
	myClient.profilepass = message.messages[5];

	if (myClient.profilelogin.Length() && myClient.profilepass.Length()) {
        PostMessage(getMainHandle(), WM_VISIT_ALOGIN, 0, 0);
	}
}

void processNotification(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришло уведомление");

	UnicodeString *buf = new UnicodeString(message.messages[0]);
	PostMessage(getMainHandle(), WM_VISIT_NOTIF, 0, (long)buf);
}

void processInfoClient(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришла информация о VNC");

//	myClient.client = message.messages[0];
//	myClient.manage = message.messages[1];
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
	addLog(TYPE_LOG_INFO, "Пришёл контакт");

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

	TURLEncoding *a = new TURLEncoding;
	UnicodeString r = a->URLDecode(message.messages[0]);
	delete a;

	myClient.contact = parseContact(r);

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
	addLog(TYPE_LOG_INFO, "Пришел статус контакта");

	Contact *c = getContact(myClient.contact, cleanPid(message.messages[0]));
	if(c != NULL){
		c->status = StrToInt(message.messages[1]);
		PostMessage(getMainHandle(), WM_VISIT_CONT, 0, (long)c);
	}
}

void processListVNC(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришла позиция vnc");

	UnicodeString *buf = new UnicodeString(message.messages[0].c_str());
	PostMessage(getMainHandle(), WM_VISIT_IVNC, 0, (long)buf);
}

void processInfoAnswer(Message message)
{
	addLog(TYPE_LOG_INFO, "Пришла информация о контакте");

	UnicodeString **buf = new UnicodeString*[2];

	buf[0] = new UnicodeString(message.messages[2].c_str());
	buf[1] = new UnicodeString(message.messages[3].c_str());

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
