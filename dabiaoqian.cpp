#include "dabiaoqian.h"
#include<iostream>
#include <QMessageBox> 
#include <QTextStream>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <global.h>


#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

dabiaoqian::dabiaoqian(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	Init();
}

QString dabiaoqian::ReadConf()
{
	QFile f("d.conf");
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{		
		QMessageBox::information(NULL, "打开配置文件", "打开配置文件d.conf失败", QMessageBox::Yes | QMessageBox::No);
		exit(0);
	}

	QTextStream txtInput(&f);
	QString lineStr;
	while (!txtInput.atEnd())
	{
		QString temp = txtInput.readLine();
		lineStr += temp;
	}

	f.close();

	return lineStr;

}
void dabiaoqian::InitCombox()
{

}


void dabiaoqian::ReadSignatue()
{
	//DWORD *ar=0;
	//DWORD d = GetAllPresentDisks(&ar);
	//QString curDir = QDir::currentPath();
	QString runPath = QCoreApplication::applicationDirPath();
	QChar curpf = runPath.at(0);

	FindAllDrivers(sigmap);
	for (std::map<std::wstring, ULONG>::iterator it = sigmap.begin(); it != sigmap.end(); it++)
	{
		std::wstring key = it->first;
		ulong signatrue = it->second;

		QString ky = QString::fromStdWString(key);
		QChar kpf = ky.at(0);
		if (curpf == kpf)
		{
			cur_signature = signatrue;
			ui.cursig_lineEdit->setText(QString::number(cur_signature));
			break;
		}
		int k = 0;
		k++;

		//key += l":";
		//QString str = QString::fromstdwstring(key);			
		//str = str + qstring::number(signatrue);
		//ui.chac_diskid_combobox->additem(str);
		//int k = 0;
		//k++;
	}

	//connect(ui.chac_diskid_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ComboxSigChange(int)));

	//ui.chac_diskid_comboBox->setVisible(false);
	//ui.chac_diskid_label->setVisible(false);
}

QString dabiaoqian::GetSignatureJson()
{
	QJsonObject json;
	json.insert("signature",(qint64)cur_signature);

	//json.insert("utype",cur_type);

	QJsonDocument document;
	document.setObject(json);
	QByteArray byte_array = document.toJson(QJsonDocument::Compact);
	QString json_str(byte_array);

	return json_str;
}

bool dabiaoqian::DealResponse(QString str,int& line_code,QString& line_utype)
{
	QJsonParseError complex_json_error;
	QJsonDocument complex_parse_doucment = QJsonDocument::fromJson(str.toUtf8(), &complex_json_error);
	int i = 0;
	if (complex_json_error.error == QJsonParseError::NoError)
	{
		QJsonObject jsonObject = complex_parse_doucment.object();
		if (jsonObject.contains("code"))
		{
			QJsonValue value = jsonObject.value("code");
			if (value.isDouble())
			{
				line_code = value.toVariant().toInt();
				if (0 != line_code) {
					return false;
				}
			}
		}
		if (jsonObject.contains("utype"))
		{
			QJsonValue value = jsonObject.value("utype");
			if (value.isString())
			{
				line_utype = value.toString();
			}

		}
	}
	else
	{
		QMessageBox::information(NULL, "服务器响应错误", "非法json内容！", QMessageBox::Yes | QMessageBox::No);
		exit(0);
		return false;
	}
	return true;
}
void dabiaoqian::UploadResponse(QNetworkReply* reply)
{
	QVariant statusCodeV =
		reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

	if(200!= statusCodeV){	
		QString str = QString::number(statusCodeV.toInt());
		QMessageBox::information(NULL, "查询标签响应", "查询标签失败,httperr:"+ str, QMessageBox::Yes);
		//return;
	}

	// Or the target URL if it was a redirect:  
	QVariant redirectionTargetUrl =
		reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	// see CS001432 on how to handle this  

	// no error received?  
	if (reply->error() == QNetworkReply::NoError)
	{
		// read data from QNetworkReply here  

		// Example 1: Creating QImage from the reply  
		//QImageReader imageReader(reply);  
		//QImage pic = imageReader.read();  

		// Example 2: Reading bytes form the reply  
		QByteArray bytes = reply->readAll();  // bytes  
		//QString string(bytes); // string  
		QString string = QString::fromUtf8(bytes);
		int line_code;
		QString line_utype;
		DealResponse(string, line_code, line_utype);
		ui.lineEdit_tagtype->setText(line_utype);
		//QMessageBox::information(NULL, "查询标签响应", string, QMessageBox::Yes | QMessageBox::No);
		//ui->textBrowser->setText(string);
	}
	// Some http error received  
	else
	{
		// handle errors here  
	}

	// We receive ownership of the reply object  
	// and therefore need to handle deletion.
	reply->deleteLater();
}
void dabiaoqian::UploadSignatureAndType()
{
	QString js = GetSignatureJson();

	QString cur = ui.curlog_textEdit->toPlainText();
	QString now = cur + js + "\r\n---------------------\r\n";
	ui.curlog_textEdit->setText(now);

	namUpload = new QNetworkAccessManager(this);
	QObject::connect(namUpload, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(UploadResponse(QNetworkReply*)));

	QString str = domain;
	QUrl url(str+ QString("only_query_uType.php?XDEBUG_SESSION_START=PHPSTORM"));
	QNetworkRequest request= QNetworkRequest(url);
	request.setUrl(url);
	request.setHeader(QNetworkRequest::CookieHeader, "XDEBUG_SESSION=PHPSTORM");
	QNetworkReply* reply = namUpload->post(request, js.toUtf8());
}

void dabiaoqian::OnClickNameType()
{
	UploadSignatureAndType();
}

void dabiaoqian::CheckResponse(QNetworkReply* reply)
{
	QVariant statusCodeV =
		reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
	// Or the target URL if it was a redirect:  
	QVariant redirectionTargetUrl =
		reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	// see CS001432 on how to handle this  

	// no error received?  
	if (reply->error() == QNetworkReply::NoError)
	{
		// read data from QNetworkReply here  

		// Example 1: Creating QImage from the reply  
		//QImageReader imageReader(reply);  
		//QImage pic = imageReader.read();  

		// Example 2: Reading bytes form the reply  
		QByteArray bytes = reply->readAll();  // bytes
											  //QString string(bytes); // string
		QString string = QString::fromUtf8(bytes);

		//ui->textBrowser->setText(string);
	}
	// Some http error received  
	else
	{
		// handle errors here  
	}

	// We receive ownership of the reply object  
	// and therefore need to handle deletion.
	reply->deleteLater();
}
//void dabiaoqian::OnClickCheck()
//{
//	QString js = GetSignatureJson();
//
//	namCheck = new QNetworkAccessManager(this);
//	QObject::connect(namCheck, SIGNAL(finished(QNetworkReply*)),
//		this, SLOT(CheckResponse(QNetworkReply*)));
//	QUrl url("http://checkbiaoqian.cow8.cn/checkdabiaoqian.php");
//	
//	QNetworkReply* reply = namCheck->post(QNetworkRequest(url), js.toUtf8());
//}

void dabiaoqian::checkDate() {

	QDate ex = QDate::fromString(expire, "yyyyMMdd");
	
	QDate nw = QDate::currentDate();
	
	if (nw > ex) {
		QMessageBox::information(NULL, "请支付三期款", "请支付三期款,谢谢", QMessageBox::Yes | QMessageBox::No);
		exit(0);
	}	
}
void dabiaoqian::Init()
{	
	//checkDate();

	InitCombox();
	ReadSignatue();

	connect(ui.nameType_pushButton, SIGNAL(clicked()), this, SLOT(OnClickNameType()));
	//connect(ui.check_pushButton, SIGNAL(clicked()), this, SLOT(OnClickCheck()));	
}