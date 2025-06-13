####
#	Class FileDialog
#
#	Purpose
#	-------
#
#	FileDialog's are widgets that allow one to select file names by
#	clicking on file names, directory names, filters, etc.
#
#	Standard Usage
#	--------------
#
#	F = FileDialog(widget, some_title, some_filter)
#	if F.Show() != 1:
#		F.DialogCleanup()
#	return
#		file_name = F.GetFileName()
#		F.DialogCleanup()
####

from dialog import *

class FileDialog(ModalDialog):

	#	constructor

	def __init__(self, widget, title, filter="*"):
		from os import getcwd
		from string import strip

		self.widget = widget
		self.filter = strip(filter)
		self.orig_dir = getcwd()
		self.cwd = getcwd()				#	the logical current working directory
		Dialog.__init__(self, widget)

	#	setup routine called back from Dialog

	def SetupDialog(self):

		# directory label

		self.dirFrame = Frame(self.top)
		self.dirFrame['relief'] = 'raised'
		self.dirFrame['bd']	 = '2'
		self.dirFrame.pack({'expand':'no', 'side':'top', 'fill':'both'})
		self.dirLabel = Label(self.dirFrame)
		self.dirLabel["text"] = "Directory:"
		self.dirLabel.pack({'expand':'no', 'side':'left', 'fill':'none'})

		# editable filter

		self.filterFrame = Frame(self.top)
		self.filterFrame['relief'] = 'raised'
		self.filterFrame['bd']	 = '2'
		self.filterFrame.pack({'expand':'no', 'side':'top', 'fill':'both'})
		self.filterLabel = Label(self.filterFrame)
		self.filterLabel["text"] = "Filter:"
		self.filterLabel.pack({'expand':'no', 'side':'left', 'fill':'none'})
		self.filterEntry = Entry(self.filterFrame)
		self.filterEntry.bind('<Return>', self.FilterReturnKey)
		self.filterEntry["width"]  = "40"
		self.filterEntry["relief"] = "ridge"
		self.filterEntry.pack({'expand':'yes', 'side':'right', 'fill':'x'})
		self.filterEntry.insert(0, self.filter)

		# the directory and file listboxes

		self.listBoxFrame = Frame(self.top)
		self.listBoxFrame['relief'] = 'raised'
		self.listBoxFrame['bd']	 = '2'
		self.listBoxFrame.pack({'expand':'yes', 'side' :'top',
			'pady' :'3', 'padx' :'3', 'fill' :'both'})
		self.CreateDirListBox()
		self.CreateFileListBox()
		self.UpdateListBoxes()

		# editable filename

		self.fileNameFrame = Frame(self.top)
		self.fileNameFrame.pack({'expand':'no', 'side':'top', 'fill':'both'})
		self.fileNameFrame['relief'] = 'raised'
		self.fileNameFrame['bd']	 = '2'
		self.fileNameLabel = Label(self.fileNameFrame)
		self.fileNameLabel["text"] = "File:"
		self.fileNameLabel.pack({'expand':'no', 'side':'left', 'fill':'none'})
		self.fileNameEntry = Entry(self.fileNameFrame)
		self.fileNameEntry["width"]  = "40"
		self.fileNameEntry["relief"] = "ridge"
		self.fileNameEntry.pack({'expand':'yes', 'side':'right', 'fill':'x'})
		self.fileNameEntry.bind('<Return>', self.FileNameReturnKey)

		#	buttons - ok, filter, cancel

		self.buttonFrame = Frame(self.top)
		self.buttonFrame['relief'] = 'raised'
		self.buttonFrame['bd']	 = '2'
		self.buttonFrame.pack({'expand':'no', 'side':'top', 'fill':'x'})
		self.okButton = Button(self.buttonFrame)
		self.okButton["text"]	  = "OK"
		self.okButton["command"]   = self.OkPressed
		self.okButton.pack({'expand':'yes', 'pady':'2', 'side':'left'})
		self.filterButton = Button(self.buttonFrame)
		self.filterButton["text"]	  = "FILTER"
		self.filterButton["command"]   = self.FilterPressed
		self.filterButton.pack({'expand':'yes', 'pady':'2', 'side':'left'})
		button = Button(self.buttonFrame)
		button["text"] = "CANCEL"
		button["command"] = self.CancelPressed
		button.pack({'expand':'yes', 'pady':'2', 'side':'left'})

	#	create the directory list box

	def CreateDirListBox(self):
		frame = Frame(self.listBoxFrame)
		frame.pack({'expand':'yes', 'side' :'left', 'pady' :'1', 
			'fill' :'both'})
		frame['relief'] = 'raised'
		frame['bd']	 = '2'
		filesFrame = Frame(frame)
		filesFrame['relief'] = 'raised'
		filesFrame['bd']	 = '2'
		filesFrame.pack({'side':'top', 'expand':'no', 'fill':'x'})
		label = Label(filesFrame)
		label['text'] = 'Directories:'
		label.pack({'side':'left', 'expand':'yes', 'anchor':'w',
			'fill':'none'})
		scrollBar = Scrollbar(frame, {'orient':'vertical'})
		scrollBar.pack({'expand':'yes', 'side':'right', 'fill':'y'})
		self.dirLb = Listbox(frame, {'yscroll':scrollBar.set})
		self.dirLb.pack({'expand':'yes', 'side' :'top', 'pady' :'1', 
			'fill' :'y'})
		self.dirLb.bind('<Double-Button-1>', self.DoDoubleClickDir)
		scrollBar['command'] = self.dirLb.yview

	#	create the files list box

	def CreateFileListBox(self):
		frame = Frame(self.listBoxFrame)
		frame['relief'] = 'raised'
		frame['bd']	 = '2'
		frame.pack({'expand':'yes', 'side' :'left', 'pady' :'1', 'padx' :'5', 
			'fill' :'both'})
		filesFrame = Frame(frame)
		filesFrame['relief'] = 'raised'
		filesFrame['bd']	 = '2'
		filesFrame.pack({'side':'top', 'expand':'no', 'fill':'x'})
		label = Label(filesFrame)
		label['text'] = 'Files:'
		label.pack({'side':'left', 'expand':'yes', 'anchor':'w', 
			'fill':'none'})
		scrollBar = Scrollbar(frame, {'orient':'vertical'})
		scrollBar.pack({'side':'right', 'fill':'y'})
		self.fileLb = Listbox(frame, {'yscroll':scrollBar.set})
		self.fileLb.pack({'expand':'yes', 'side' :'top', 'pady' :'1', 
			'fill' :'both'})
		self.fileLb.bind('<1>', self.DoSelection)
		self.fileLb.bind('<Double-Button-1>', self.DoDoubleClickFile)
		scrollBar['command'] = self.fileLb.yview

	#	update the listboxes and directory label after a change of directory

	def UpdateListBoxes(self):
		from os import listdir
		from posixpath import isfile, isdir, join, basename
		from commands import getoutput
		from string import splitfields

		cwd = self.cwd
		self.fileLb.delete(0, self.fileLb.size())
		filter = self.filterEntry.get()
		# '*' will list recurively, we don't want that.
		if filter == '*':
			filter = ''
		cmd = "/bin/ls " + join(cwd, filter)
		cmdOutput = getoutput(cmd)
		files = splitfields(cmdOutput, "\n")
		files.sort()
		for i in range(len(files)):
			if isfile(join(cwd, files[i])):
				self.fileLb.insert('end', basename(files[i]))
		self.dirLb.delete(0, self.dirLb.size())
		files = listdir(cwd)
		files.sort()
		for i in range(len(files)):
			if isdir(join(cwd, files[i])):
				self.dirLb.insert('end', files[i])
		self.dirLabel['text'] = "Directory:" + self.cwd_print()

	#	selection handlers

	def DoSelection(self, event):
		from posixpath import join
		lb = event.widget
		field = self.fileNameEntry
		field.delete(0, AtEnd())
		field.insert(0, join(self.cwd_print(), lb.get(lb.nearest(event.y))))
		lb.select_clear()
		lb.select_from(lb.nearest(event.y))
		
	def DoDoubleClickDir(self, event):
		from posixpath import join
		lb = event.widget
		self.cwd = join(self.cwd, lb.get(lb.nearest(event.y)))
		self.UpdateListBoxes()

	def DoDoubleClickFile(self, event):
		self.OkPressed()

	def OkPressed(self):
		self.TerminateDialog(1)

	def FileNameReturnKey(self, event):
		from posixpath import isabs, expanduser, join
		from string import strip
		#	if its a relative path then include the cwd in the name
		name = strip(self.fileNameEntry.get())
		if not isabs(expanduser(name)):
			self.fileNameEntry.delete(0, 'end')
			self.fileNameEntry.insert(0, join(self.cwd_print(), name))
		self.okButton.flash()
		self.OkPressed()
	
	def FilterReturnKey(self, event):
		from string import strip
		filter = strip(self.filterEntry.get())
		self.filterEntry.delete(0, 'end')
		self.filterEntry.insert(0, filter)
		self.filterButton.flash()
		self.UpdateListBoxes()

	def FilterPressed(self):
		self.UpdateListBoxes()

	def CancelPressed(self):
		self.TerminateDialog(0)

	def GetFileName(self):
		return self.fileNameEntry.get()
		
	#	return the logical current working directory in a printable form
	#	ie. without all the X/.. pairs. The easiest way to do this is to
	#	chdir to cwd and get the path there.

	def cwd_print(self):
		from os import chdir, getcwd
		chdir(self.cwd)
		p = getcwd()
		chdir(self.orig_dir)
		return p

####
#	Class LoadFileDialog
#
#	Purpose
#	-------
#
#	Specialisation of FileDialog for loading files.
####

class LoadFileDialog(FileDialog):

	def __init__(self, master, title, filter):
		FileDialog.__init__(self, master, title, filter)
		self.top.title(title)

	def OkPressed(self):
		from utils import file_exists
		from alert import ErrorDialog
		fileName = self.GetFileName()
		if file_exists(fileName) == 0:
			str = 'File ' + fileName + ' not found.'
			errorDlg = ErrorDialog(self.top, str)
			errorDlg.Show()
			errorDlg.DialogCleanup()
			return
		FileDialog.OkPressed(self)

####
#	Class SaveFileDialog
#
#	Purpose
#	-------
#
#	Specialisation of FileDialog for saving files.
####

class SaveFileDialog(FileDialog):

	def __init__(self, master, title, filter):
		FileDialog.__init__(self, master, title, filter)
		self.top.title(title)

	def OkPressed(self):
		from utils import file_exists
		from alert import WarningDialog
		fileName = self.GetFileName()
		if file_exists(fileName) == 1:
			str = 'File ' + fileName + ' exists.\nDo you wish to overwrite it?'
			warningDlg = WarningDialog(self.top, str)
			if warningDlg.Show() == 0:
				warningDlg.DialogCleanup()
				return
			warningDlg.DialogCleanup()
		FileDialog.OkPressed(self)
