import smtplib
import os
from os.path import isfile, basename, splitext
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase

if __name__ == "__main__":
	to_email = input("to: ")
	from_email = input("from: ")
	password = input("password: ")

	mime = MIMEMultipart()
	mime['From'] = from_email
	mime['To'] = to_email

	user_input = os.getcwd()
	directory = os.listdir(user_input)
	searchstring = input('keyword to find in text files: ')
	for fname in directory:
		if fname.endswith('.txt') and os.path.isfile(user_input + os.sep + fname):
			f = open(user_input + os.sep + fname, 'r')
			if searchstring in f.read():
				print('found string in file %s' % fname)
				with open(fname, 'rb') as file:
					part = MIMEApplication(file.read(), Name=basename(fname))
				part.add_header('Content-Disposition', 'attachment; filename="{}"'.format(fname))
				mime.attach(part)
			f.close()
	try:
		server_ssl = smtplib.SMTP_SSL('smtp.gmail.com', 465)
		print("smtp connected")
		server_ssl.ehlo()
		server_ssl.login(from_email, password)
		print("logined")
		server_ssl.sendmail(mime['From'], mime['To'], mime.as_string())
		server_ssl.close()
		print('Email sent!')
	except Exception:
		print("Samething went wrong!")
