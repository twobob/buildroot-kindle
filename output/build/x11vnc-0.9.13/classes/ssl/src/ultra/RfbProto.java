//  Copyright (C) 2002-2004 Ultr@VNC Team.  All Rights Reserved.
//  Copyright (C) 2004 Kenn Min Chong, John Witchel.  All Rights Reserved.
//  Copyright (C) 2001,2002 HorizonLive.com, Inc.  All Rights Reserved.
//  Copyright (C) 2001,2002 Constantin Kaplinsky.  All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation.  All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
//
//  This is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this software; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

//
// RfbProto.java
//4/19/04

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.net.Socket;
import java.util.*;
import java.util.zip.*;
import java.text.DateFormat;


class RfbProto {

	final String versionMsg = "RFB 003.003\n";
	final static int ConnFailed = 0, NoAuth = 1, VncAuth = 2, MsLogon = 0xfffffffa;
	final static int VncAuthOK = 0, VncAuthFailed = 1, VncAuthTooMany = 2;

	final static int FramebufferUpdate = 0,
		SetColourMapEntries = 1,
		Bell = 2,
		ServerCutText = 3,
		rfbFileTransfer = 7;

	final int SetPixelFormat = 0,
		FixColourMapEntries = 1,
		SetEncodings = 2,
		FramebufferUpdateRequest = 3,
		KeyboardEvent = 4,
		PointerEvent = 5,
		ClientCutText = 6;

	final static int EncodingRaw = 0,
		EncodingCopyRect = 1,
		EncodingRRE = 2,
		EncodingCoRRE = 4,
		EncodingHextile = 5,
		EncodingZlib = 6,
		EncodingTight = 7,
		EncodingCompressLevel0 = 0xFFFFFF00,
		EncodingQualityLevel0 = 0xFFFFFFE0,
		EncodingXCursor = 0xFFFFFF10,
		EncodingRichCursor = 0xFFFFFF11,
		EncodingPointerPos = 0xFFFFFF18, // marscha - PointerPos
		EncodingLastRect = 0xFFFFFF20,
		EncodingNewFBSize = 0xFFFFFF21;

	final int HextileRaw = (1 << 0);
	final int HextileBackgroundSpecified = (1 << 1);
	final int HextileForegroundSpecified = (1 << 2);
	final int HextileAnySubrects = (1 << 3);
	final int HextileSubrectsColoured = (1 << 4);

	final static int TightExplicitFilter = 0x04;
	final static int TightFill = 0x08;
	final static int TightJpeg = 0x09;
	final static int TightMaxSubencoding = 0x09;
	final static int TightFilterCopy = 0x00;
	final static int TightFilterPalette = 0x01;
	final static int TightFilterGradient = 0x02;

	final static int TightMinToCompress = 12;

	// sf@2004 - FileTransfer part
	ArrayList remoteDirsList;
	ArrayList remoteDirsListInfo;
	ArrayList remoteFilesList;
	ArrayList remoteFilesListInfo;
	ArrayList a;
	ArrayList b;
	boolean fFTInit = true; // sf@2004
	boolean fFTAllowed = true;
	boolean fAbort = false;
	boolean fFileReceptionError = false;
	boolean fFileReceptionRunning = false;
	boolean inDirectory2;
	FileOutputStream fos;
	FileInputStream fis;
	String sendFileSource;
	String receivePath;
	long fileSize;
	long receiveFileSize;
	long fileChunkCounter;

	final static int sz_rfbFileTransferMsg = 12,
	// FileTransfer Content types and Params defines
	rfbDirContentRequest = 1,
	// Client asks for the content of a given Server directory
	rfbDirPacket = 2, // Full directory name or full file name.
	// Null content means end of Directory
	rfbFileTransferRequest = 3,
	// Client asks the server for the tranfer of a given file
	rfbFileHeader = 4,
	// First packet of a file transfer, containing file's features
	rfbFilePacket = 5, // One slice of the file
	rfbEndOfFile = 6,
	// End of file transfer (the file has been received or error)
	rfbAbortFileTransfer = 7,
	// The file transfer must be aborted, whatever the state
	rfbFileTransferOffer = 8,
	// The client offers to send a file to the server
	rfbFileAcceptHeader = 9, // The server accepts or rejects the file
	rfbCommand = 10,
	// The Client sends a simple command (File Delete, Dir create etc...)
	rfbCommandReturn = 11,
	//	New FT Protocole (V2) The zipped checksums of the destination file (Delta Transfer)
	rfbFileChecksums = 12,
	// The Client receives the server's answer about a simple command
	// rfbDirContentRequest client Request - content params 
	rfbRDirContent = 1, // Request a Server Directory contents
	rfbRDrivesList = 2, // Request the server's drives list
	
	// rfbDirPacket & rfbCommandReturn  server Answer - content params
	rfbADirectory = 1, // Reception of a directory name
	rfbAFile = 2, // Reception of a file name 
	rfbADrivesList = 3, // Reception of a list of drives
	rfbADirCreate = 4, // Response to a create dir command 
	rfbADirDelete = 5, // Response to a delete dir command 
	rfbAFileCreate = 6, // Response to a create file command 
	rfbAFileDelete = 7, // Response to a delete file command
	
	// rfbCommand Command - content params
	rfbCDirCreate = 1, // Request the server to create the given directory
	rfbCDirDelete = 2, // Request the server to delete the given directory
	rfbCFileCreate = 3, // Request the server to create the given file
	rfbCFileDelete = 4, // Request the server to delete the given file
	
	// Errors - content params or "size" field
	rfbRErrorUnknownCmd = 1, // Unknown FileTransfer command.
	rfbRErrorCmd = 0xFFFFFFFF,
	
	// Error when a command fails on remote side (ret in "size" field)
	sz_rfbBlockSize = 8192, // new FT protocole (v2)
	
	// Size of a File Transfer packet (before compression)
	sz_rfbZipDirectoryPrefix = 9;

	String rfbZipDirectoryPrefix = "!UVNCDIR-\0";
	// Transfered directory are zipped in a file with this prefix. Must end with "-"
	
	// End of FileTransfer part 
	
	String host;
	int port;
	Socket sock;
	DataInputStream is;
	OutputStream os;
	OutputStreamWriter osw;

	SessionRecorder rec;
	boolean inNormalProtocol = false;
	VncViewer viewer;

	// Java on UNIX does not call keyPressed() on some keys, for example
	// swedish keys To prevent our workaround to produce duplicate
	// keypresses on JVMs that actually works, keep track of if
	// keyPressed() for a "broken" key was called or not. 
	boolean brokenKeyPressed = false;

	// This will be set to true on the first framebuffer update
	// containing Zlib- or Tight-encoded data.
	boolean wereZlibUpdates = false;

	// This will be set to false if the startSession() was called after
	// we have received at least one Zlib- or Tight-encoded framebuffer
	// update.
	boolean recordFromBeginning = true;

	// This fields are needed to show warnings about inefficiently saved
	// sessions only once per each saved session file.
	boolean zlibWarningShown;
	boolean tightWarningShown;

	// Before starting to record each saved session, we set this field
	// to 0, and increment on each framebuffer update. We don't flush
	// the SessionRecorder data into the file before the second update. 
	// This allows us to write initial framebuffer update with zero
	// timestamp, to let the player show initial desktop before
	// playback.
	int numUpdatesInSession;

// begin runge/x11vnc
	int readServerDriveListCnt = -1;
	long readServerDriveListTime = 0;
// end   runge/x11vnc
	//
	// Constructor. Make TCP connection to RFB server.
	//

	RfbProto(String h, int p, VncViewer v) throws IOException {
		viewer = v;
		host = h;
		port = p;
// begin runge/x11vnc
//		sock = new Socket(host, port);
    if (! viewer.disableSSL) {
       System.out.println("new SSLSocketToMe");
       SSLSocketToMe ssl;
       try {
               ssl = new SSLSocketToMe(host, port, v);
       } catch (Exception e) {
               throw new IOException(e.getMessage());
       }

       try {
               sock = ssl.connectSock();
       } catch (Exception es) {
               throw new IOException(es.getMessage());
       }
    } else {
       sock = new Socket(host, port);
    }
// end runge/x11vnc

		is =
			new DataInputStream(
				new BufferedInputStream(sock.getInputStream(), 16384));
		os = sock.getOutputStream();
		osw = new OutputStreamWriter(sock.getOutputStream());
		inDirectory2 = false;
		a = new ArrayList();
		b = new ArrayList();
		// sf@2004
		remoteDirsList = new ArrayList();
		remoteDirsListInfo = new ArrayList();
		remoteFilesList = new ArrayList();
		remoteFilesListInfo = new ArrayList();
	
		sendFileSource = "";
	}

	void close() {
		try {
			sock.close();
			if (rec != null) {
				rec.close();
				rec = null;
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	//
	// Read server's protocol version message
	//

	int serverMajor, serverMinor;

	void readVersionMsg() throws Exception {

		byte[] b = new byte[12];

		is.readFully(b);

		if ((b[0] != 'R')
			|| (b[1] != 'F')
			|| (b[2] != 'B')
			|| (b[3] != ' ')
			|| (b[4] < '0')
			|| (b[4] > '9')
			|| (b[5] < '0')
			|| (b[5] > '9')
			|| (b[6] < '0')
			|| (b[6] > '9')
			|| (b[7] != '.')
			|| (b[8] < '0')
			|| (b[8] > '9')
			|| (b[9] < '0')
			|| (b[9] > '9')
			|| (b[10] < '0')
			|| (b[10] > '9')
			|| (b[11] != '\n')) {
			throw new Exception(
				"Host " + host + " port " + port + " is not an RFB server");
		}

		serverMajor = (b[4] - '0') * 100 + (b[5] - '0') * 10 + (b[6] - '0');
		serverMinor = (b[8] - '0') * 100 + (b[9] - '0') * 10 + (b[10] - '0');
	}

	//
	// Write our protocol version message
	//

	void writeVersionMsg() throws IOException {
		os.write(versionMsg.getBytes());
	}

	//
	// Find out the authentication scheme.
	//

	int readAuthScheme() throws Exception {
		int authScheme = is.readInt();

		switch (authScheme) {

			case ConnFailed :
				int reasonLen = is.readInt();
				byte[] reason = new byte[reasonLen];
				is.readFully(reason);
				throw new Exception(new String(reason));

			case NoAuth :
			case VncAuth :
			case MsLogon:
				return authScheme;

			default :
				throw new Exception(
					"Unknown authentication scheme from RFB server: "
						+ authScheme);

		}
	}

	//
	// Write the client initialisation message
	//

	void writeClientInit() throws IOException {
		if (viewer.options.shareDesktop) {
			os.write(1);
		} else {
			os.write(0);
		}
		viewer.options.disableShareDesktop();
	}

	//
	// Read the server initialisation message
	//

	String desktopName;
	int framebufferWidth, framebufferHeight;
	int bitsPerPixel, depth;
	boolean bigEndian, trueColour;
	int redMax, greenMax, blueMax, redShift, greenShift, blueShift;

	void readServerInit() throws IOException {
		framebufferWidth = is.readUnsignedShort();
		framebufferHeight = is.readUnsignedShort();
		bitsPerPixel = is.readUnsignedByte();
		depth = is.readUnsignedByte();
		bigEndian = (is.readUnsignedByte() != 0);
		trueColour = (is.readUnsignedByte() != 0);
		redMax = is.readUnsignedShort();
		greenMax = is.readUnsignedShort();
		blueMax = is.readUnsignedShort();
		redShift = is.readUnsignedByte();
		greenShift = is.readUnsignedByte();
		blueShift = is.readUnsignedByte();
		byte[] pad = new byte[3];
		is.readFully(pad);
		int nameLength = is.readInt();
		byte[] name = new byte[nameLength];
		is.readFully(name);
		desktopName = new String(name);

		inNormalProtocol = true;
	}

	//
	// Create session file and write initial protocol messages into it.
	//

	void startSession(String fname) throws IOException {
		rec = new SessionRecorder(fname);
		rec.writeHeader();
		rec.write(versionMsg.getBytes());
		rec.writeIntBE(NoAuth);
		rec.writeShortBE(framebufferWidth);
		rec.writeShortBE(framebufferHeight);
		byte[] fbsServerInitMsg =
			{
				32,
				24,
				0,
				1,
				0,
				(byte) 0xFF,
				0,
				(byte) 0xFF,
				0,
				(byte) 0xFF,
				16,
				8,
				0,
				0,
				0,
				0 };
		rec.write(fbsServerInitMsg);
		rec.writeIntBE(desktopName.length());
		rec.write(desktopName.getBytes());
		numUpdatesInSession = 0;

		if (wereZlibUpdates)
			recordFromBeginning = false;

		zlibWarningShown = false;
		tightWarningShown = false;
	}

	//
	// Close session file.
	//

	void closeSession() throws IOException {
		if (rec != null) {
			rec.close();
			rec = null;
		}
	}

	//
	// Set new framebuffer size
	//

	void setFramebufferSize(int width, int height) {
		framebufferWidth = width;
		framebufferHeight = height;
	}

	//
	// Read the server message type
	//

	int readServerMessageType() throws IOException {
		int msgType;
		try {
			msgType = is.readUnsignedByte();
		} catch (Exception e) {
			viewer.disconnect();
			return -1;
		}

		// If the session is being recorded:
		if (rec != null) {
			if (msgType == Bell) { // Save Bell messages in session files.
				rec.writeByte(msgType);
				if (numUpdatesInSession > 0)
					rec.flush();
			}
		}

		return msgType;
	}

	//
	// Read a FramebufferUpdate message
	//

	int updateNRects;

	void readFramebufferUpdate() throws IOException {
		is.readByte();
		updateNRects = is.readUnsignedShort();

		// If the session is being recorded:
		if (rec != null) {
			rec.writeByte(FramebufferUpdate);
			rec.writeByte(0);
			rec.writeShortBE(updateNRects);
		}

		numUpdatesInSession++;
	}

	// Read a FramebufferUpdate rectangle header

	int updateRectX, updateRectY, updateRectW, updateRectH, updateRectEncoding;

	void readFramebufferUpdateRectHdr() throws Exception {
		updateRectX = is.readUnsignedShort();
		updateRectY = is.readUnsignedShort();
		updateRectW = is.readUnsignedShort();
		updateRectH = is.readUnsignedShort();
		updateRectEncoding = is.readInt();

		if (updateRectEncoding == EncodingZlib
			|| updateRectEncoding == EncodingTight)
			wereZlibUpdates = true;

		// If the session is being recorded:
		if (rec != null) {
			if (numUpdatesInSession > 1)
				rec.flush(); // Flush the output on each rectangle.
			rec.writeShortBE(updateRectX);
			rec.writeShortBE(updateRectY);
			rec.writeShortBE(updateRectW);
			rec.writeShortBE(updateRectH);
			if (updateRectEncoding == EncodingZlib && !recordFromBeginning) {
				// Here we cannot write Zlib-encoded rectangles because the
				// decoder won't be able to reproduce zlib stream state.
				if (!zlibWarningShown) {
					System.out.println(
						"Warning: Raw encoding will be used "
							+ "instead of Zlib in recorded session.");
					zlibWarningShown = true;
				}
				rec.writeIntBE(EncodingRaw);
			} else {
				rec.writeIntBE(updateRectEncoding);
				if (updateRectEncoding == EncodingTight
					&& !recordFromBeginning
					&& !tightWarningShown) {
					System.out.println(
						"Warning: Re-compressing Tight-encoded "
							+ "updates for session recording.");
					tightWarningShown = true;
				}
			}
		}

		if (updateRectEncoding == EncodingLastRect
			|| updateRectEncoding == EncodingNewFBSize)
			return;

		if (updateRectX + updateRectW > framebufferWidth
			|| updateRectY + updateRectH > framebufferHeight) {
			throw new Exception(
				"Framebuffer update rectangle too large: "
					+ updateRectW
					+ "x"
					+ updateRectH
					+ " at ("
					+ updateRectX
					+ ","
					+ updateRectY
					+ ")");
		}
	}

	// Read CopyRect source X and Y.

	int copyRectSrcX, copyRectSrcY;

	void readCopyRect() throws IOException {
		copyRectSrcX = is.readUnsignedShort();
		copyRectSrcY = is.readUnsignedShort();

		// If the session is being recorded:
		if (rec != null) {
			rec.writeShortBE(copyRectSrcX);
			rec.writeShortBE(copyRectSrcY);
		}
	}

	//
	// Read a ServerCutText message
	//

	String readServerCutText() throws IOException {
		byte[] pad = new byte[3];
		is.readFully(pad);
		int len = is.readInt();
		byte[] text = new byte[len];
		is.readFully(text);
		return new String(text);
	}

	//
	// Read an integer in compact representation (1..3 bytes).
	// Such format is used as a part of the Tight encoding.
	// Also, this method records data if session recording is active and
	// the viewer's recordFromBeginning variable is set to true.
	//

	int readCompactLen() throws IOException {
		int[] portion = new int[3];
		portion[0] = is.readUnsignedByte();
		int byteCount = 1;
		int len = portion[0] & 0x7F;
		if ((portion[0] & 0x80) != 0) {
			portion[1] = is.readUnsignedByte();
			byteCount++;
			len |= (portion[1] & 0x7F) << 7;
			if ((portion[1] & 0x80) != 0) {
				portion[2] = is.readUnsignedByte();
				byteCount++;
				len |= (portion[2] & 0xFF) << 14;
			}
		}

		if (rec != null && recordFromBeginning)
			for (int i = 0; i < byteCount; i++)
				rec.writeByte(portion[i]);

		return len;
	}

	//Author: Kenn Min Chong/////////////////////////////////////////////

	//Read/Write a rfbFileTransferMsg
	/*typedef struct _rfbFileTransferMsg {
	    CARD8 type;			// always rfbFileTransfer
	    CARD8 contentType;  // See defines below
	    CARD16 contentParam;// Other possible content classification (Dir or File name, etc..)
		CARD32 size;		// FileSize or packet index or error or other 
	    CARD32 length;
	    // followed by data char text[length]
	} rfbFileTransferMsg;
	*/

	//	Parsing Rfb message to see what type 

	void readRfbFileTransferMsg() throws IOException
	{
		int contentType = is.readUnsignedByte();
		int contentParamT = is.readUnsignedByte();
		int contentParam = contentParamT;
		contentParamT = is.readUnsignedByte();
		contentParamT = contentParamT << 8;
		contentParam = contentParam | contentParamT;
//System.out.println("FTM: contentType " + contentType + " contentParam " + contentParam);
		if (contentType == rfbRDrivesList || contentType == rfbDirPacket)
		{
			readDriveOrDirectory(contentParam);
		}
		else if (contentType == rfbFileHeader)
		{
			receiveFileHeader();
		}
		else if (contentType == rfbFilePacket)
		{
			receiveFileChunk();
		}
		else if (contentType == rfbEndOfFile)
		{
			endOfReceiveFile(true); // Ok
		}
		else if (contentType == rfbAbortFileTransfer)
		{
			System.out.println("rfbAbortFileTransfer: fFileReceptionRunning="
			    + fFileReceptionRunning + " fAbort="
			    + fAbort + " fFileReceptionError="
			    + fFileReceptionError);
			if (fFileReceptionRunning)
			{
				endOfReceiveFile(false); // Error
			}
			else
			{
				// sf@2004 - Todo: Add TestPermission 
				// System.out.println("File Transfer Aborted!");

				// runge: seems like we must at least read the remaining
				// 8 bytes of the header, right?
				int size = is.readInt();
				int length = is.readInt();
			}
			
		}
		else if (contentType == rfbCommandReturn)
		{
			createDirectoryorDeleteFile(contentParam);
		}
		else if (contentType == rfbFileAcceptHeader)
		{
			sendFile();
		}
		else if (contentType == rfbFileChecksums)
		{
			ReceiveDestinationFileChecksums();
		}
		else
		{
			System.out.println("ContentType: " + contentType);
		}
//System.out.println("FTM: done");
	}

	//Refactored from readRfbFileTransferMsg()
	public void createDirectoryorDeleteFile(int contentParam)
		throws IOException {
		if (contentParam == rfbADirCreate)
		{
			createRemoteDirectoryFeedback();
		}
		else if (contentParam == rfbAFileDelete)
		{
			deleteRemoteFileFeedback();
		}
	}

	//Refactored from readRfbFileTransferMsg()
	public void readDriveOrDirectory(int contentParam) throws IOException {
//System.out.println("RDOD: " + contentParam + " " + inDirectory2);
		if (contentParam == rfbADrivesList)
		{
			readFTPMsgDriveList();
		}
		else if (contentParam == rfbADirectory && !inDirectory2)
		{
			inDirectory2 = true;
			readFTPMsgDirectoryList();
		}
		else if (contentParam == rfbADirectory && inDirectory2)
		{
			readFTPMsgDirectoryListContent();
		}
		else if (contentParam == 0)
		{
			readFTPMsgDirectoryListEndContent();
			inDirectory2 = false;
		}
		else
		{
			System.out.println("ContentParam: " + contentParam);
		}
	}

	// Internally used. Write an Rfb message to the server
	void writeRfbFileTransferMsg(
		int contentType,
		int contentParam,
		long size, // 0 : compression not supported - 1 : compression supported
		long length,
		String text) throws IOException
	{
		byte b[] = new byte[12];
		byte byteArray[];

		if (viewer.dsmActive) {
			// need to send the rfbFileTransfer msg type twice for the plugin...
			byte b2[] = new byte[1];
			b2[0] = (byte) rfbFileTransfer;
			os.write(b2);
		}

		b[0] = (byte) rfbFileTransfer;
		b[1] = (byte) contentType;
		b[2] = (byte) contentParam;

		byte by = 0;
		long c = 0;

		c = size & 0xFF000000;
		by = (byte) (c >>> 24);
		b[4] = by;
		c = size & 0xFF0000;
		by = (byte) (c >>> 16);
		b[5] = by;
		c = size & 0xFF00;
		by = (byte) (c >>> 8);
		b[6] = by;
		c = size & 0xFF;
		by = (byte) c;
		b[7] = by;

		if (text != null) {
			byte byteArray0[] = text.getBytes();
			int maxc = max_char(text);
			if (maxc > 255) {
				System.out.println("writeRfbFileTransferMsg: using getBytes(\"UTF-8\")");
				byteArray0 = text.getBytes("UTF-8");
			} else if (maxc > 127) {
				System.out.println("writeRfbFileTransferMsg: using getBytes(\"ISO-8859-1\")");
				byteArray0 = text.getBytes("ISO-8859-1");
			}
			byteArray = new byte[byteArray0.length + 1];
			for (int i = 0; i < byteArray0.length; i++) {
				byteArray[i] = byteArray0[i];
			}
			byteArray[byteArray.length - 1] = 0;
System.out.println("writeRfbFileTransferMsg: length: " + length + " -> byteArray.length: " + byteArray.length);

			// will equal length for ascii, ISO-8859-1, more for UTF-8
			length = byteArray.length;

			//length++;	// used to not include null byte at end.
		} else {
			String moo = "moo";
			byteArray = moo.getBytes();
		}

		c = length & 0xFF000000;
		by = (byte) (c >>> 24);
		b[8] = by;
		c = length & 0xFF0000;
		by = (byte) (c >>> 16);
		b[9] = by;
		c = length & 0xFF00;
		by = (byte) (c >>> 8);
		b[10] = by;
		c = length & 0xFF;
		by = (byte) c;
		b[11] = by;
		os.write(b);

//System.out.println("size: " + size + " length: " + length + " text: " + text);
		

		if (text != null)
		{
			os.write(byteArray);
		}
	}

	int max_char(String text) {
		int maxc = 0;
		char chars[] = text.toCharArray();
		for (int n = 0; n < chars.length; n++) {
			if ((int) chars[n] > maxc) {
				maxc = (int) chars[n];
			}
		}
		return maxc;
	}

	String guess_encoding(char[] chars) {
		boolean saw_high_char = false;

		for (int i = 0; i < chars.length; i++) {
			if (chars[i] == '\0') {
				break;
			}
			if (chars[i] >= 128) {
				saw_high_char = true;
				break;
			}
		}
		if (!saw_high_char) {
			return "ASCII";
		}
		char prev = 1;
		boolean valid_utf8 = true;
		int n = 0;
		for (int i = 0; i < chars.length; i++) {
			if (chars[i] == '\0') {
				break;
			}
			char c = chars[i];
			if (prev < 128 && c >= 128) {
				if (c >> 5 == 0x6) {
					n = 1;
				} else if (c >> 4 == 0xe) {
					n = 2;
				} else if (c >> 3 == 0x1e) {
					n = 3;
				} else if (c >> 2 == 0x3e) {
					n = 4;
				} else {
					valid_utf8 = false;
					break;
				}
			} else {
				if (n > 0) {
					if (c < 128) {
						valid_utf8 = false;
						break;
					}
					n--;
				}
			}

			prev = c;
		}
		if (valid_utf8) {
			return "UTF-8";
		} else {
			return "ISO-8859-1";
		}
	}


	//Internally used. Write an rfb message to the server for sending files ONLY 
	int writeRfbFileTransferMsgForSendFile(
		int contentType,
		int contentParam,
		long size,
		long length,
		String source
		) throws IOException
	{
		File f = new File(source);
		fis = new FileInputStream(f);
		byte byteBuffer[] = new byte[sz_rfbBlockSize]; 
		int bytesRead = fis.read(byteBuffer);
		long counter=0;
		boolean fError = false;
		
		// sf@ - Manage compression
		boolean fCompress = true;
		Deflater myDeflater = new Deflater();
		byte[] CompressionBuffer = new byte[sz_rfbBlockSize + 1024];
		int compressedSize = 0;
	
		while (bytesRead!=-1)
		{
			counter += bytesRead;
			myDeflater.setInput(byteBuffer, 0, bytesRead);
			myDeflater.finish();
			compressedSize = myDeflater.deflate(CompressionBuffer);
			myDeflater.reset();
			// If the compressed data is larger than the original one, we're dealing with
			// already compressed data
			if (compressedSize > bytesRead)
				fCompress = false;
			this.writeRfbFileTransferMsg(
				contentType,
				contentParam,
				(fCompress ? 1 : 0), 
// RUNGE				(fCompress ? compressedSize-1 : bytesRead-1),
				(fCompress ? compressedSize : bytesRead),
				null
			);
			// Todo: Test write error !
			os.write(fCompress ? CompressionBuffer : byteBuffer, 0, fCompress ? compressedSize : bytesRead);
			
			// Todo: test read error !
			bytesRead = fis.read(byteBuffer);
			
			// viewer.ftp.connectionStatus.setText("Sent: "+ counter + " bytes of "+ f.length() + " bytes");
			viewer.ftp.jProgressBar.setValue((int)((counter * 100) / f.length()));
			viewer.ftp.connectionStatus.setText(">>> Sending File: " + source + " - Size: " + f.length() + " bytes - Progress: " + ((counter * 100) / f.length()) + "%");
			
			if (fAbort == true)
			{
				fAbort = false;
				fError = true;
				break;
			}
			try
			{
		        Thread.sleep(5);
		    }
			catch(InterruptedException e)
			{
		        System.err.println("Interrupted");
		    }				
		}
		
		writeRfbFileTransferMsg(fError ? rfbAbortFileTransfer : rfbEndOfFile, 0, 0, 0, null);
		fis.close();
		return (fError ? -1 : 1);
	}

	//This method is internally used to send the file to the server once the server is ready
	void sendFile()
	{
		try
		{
			viewer.ftp.disableButtons();
			int size = is.readInt();
			int length = is.readInt();
			for (int i = 0; i < length; i++)
			{
				System.out.print((char) is.readUnsignedByte());
			}
			System.out.println("");

			if (size == rfbRErrorCmd || size == -1) {
				viewer.ftp.enableButtons();
				viewer.ftp.connectionStatus.setText("Remote file not available for writing.");
				viewer.ftp.historyComboBox.insertItemAt(new String(" > Error - Remote file not available for writing."), 0);
				viewer.ftp.historyComboBox.setSelectedIndex(0);
				return;
			}
			
			int ret = writeRfbFileTransferMsgForSendFile(rfbFilePacket, 0, 0, 0, sendFileSource);
	
			viewer.ftp.refreshRemoteLocation();
			if (ret != 1)
			{
				viewer.ftp.connectionStatus.setText(" > Error - File NOT sent");
				viewer.ftp.historyComboBox.insertItemAt(new String(" > Error - File: <" + sendFileSource)
				    + "> was not correctly sent (aborted or error). Data may still be buffered/in transit. Wait for remote listing...",0);
			}
			else
			{
				viewer.ftp.connectionStatus.setText(" > File sent");
				viewer.ftp.historyComboBox.insertItemAt(new String(" > File: <" + sendFileSource)
				    + "> was sent to Remote Machine. Note: data may still be buffered/in transit. Wait for remote listing...",0);
			}
			viewer.ftp.historyComboBox.setSelectedIndex(0);
			viewer.ftp.enableButtons();
		}
		catch (IOException e)
		{
			System.err.println(e);
		}
	}

	//Call this method to send a file from local pc to server
	void offerLocalFile(String source, String destinationPath)
	{
		try
		{
			sendFileSource = source;
			File f = new File(source);
			// sf@2004 - Add support for huge files
			long lSize = f.length();
			int iLowSize = (int)(lSize & 0x00000000FFFFFFFF); 
			int iHighSize = (int)(lSize >> 32);
			
			String temp = destinationPath + f.getName();
			writeRfbFileTransferMsg(
									rfbFileTransferOffer,
									0,
									iLowSize, // f.length(),
									temp.length(),
									temp);
			
			// sf@2004 - Send the high part of the size			
			byte b[] = new byte[4];
			byte by = 0;
			long c = 0;
			c = iHighSize & 0xFF000000;
			by = (byte) (c >>> 24);
			b[0] = by;
			c = iHighSize & 0xFF0000;
			by = (byte) (c >>> 16);
			b[1] = by;
			c = iHighSize & 0xFF00;
			by = (byte) (c >>> 8);
			b[2] = by;
			c = iHighSize & 0xFF;
			by = (byte) c;
			b[3] = by;			
			os.write(b); 
		}
		catch (IOException e)
		{
			System.err.println(e);
		}
	}

	//Internally used.
	//Handles acknowledgement that the file has been deleted on the server
	void deleteRemoteFileFeedback() throws IOException
	{
		int ret = is.readInt();
		int length = is.readInt();
		String f = "";
		for (int i = 0; i < length; i++)
		{
			f += (char)is.readUnsignedByte();
		}
		
		viewer.ftp.refreshRemoteLocation();	
		if (ret == -1) {
			viewer.ftp.historyComboBox.insertItemAt(new String(" > ERROR Could not Delete File On Remote Machine: "),0);
		} else {
			viewer.ftp.historyComboBox.insertItemAt(new String(" > Deleted File On Remote Machine: " + f.substring(0, f.length()-1)),0);
		}
		viewer.ftp.historyComboBox.setSelectedIndex(0);
	}

	//Call this method to delete a file at server
	void deleteRemoteFile(String text)
	{
		try
		{
			String temp = text;
			writeRfbFileTransferMsg(rfbCommand, rfbCFileDelete, 0, temp.length(), temp);
		}
		catch (IOException e)
		{
			System.err.println(e);
		}
	}

	//Internally used.
	// Handles acknowledgement that the directory has been created on the server
	void createRemoteDirectoryFeedback() throws IOException
	{
		int ret = is.readInt();
		int length = is.readInt();
		String f="";
		for (int i = 0; i < length; i++)
		{
			f += (char)is.readUnsignedByte();
		}
		viewer.ftp.refreshRemoteLocation();	
		if (ret == -1) {
			viewer.ftp.historyComboBox.insertItemAt(new String(" > ERROR Could not Create Directory on Remote Machine."),0);
		} else {
			viewer.ftp.historyComboBox.insertItemAt(new String(" > Created Directory on Remote Machine: " + f.substring(0, f.length()-1)),0);
		}
		viewer.ftp.historyComboBox.setSelectedIndex(0);
	}

	//Call this method to create a directory at server
	void createRemoteDirectory(String text)
	{
		try
		{
			String temp = text;
			writeRfbFileTransferMsg(rfbCommand, rfbCDirCreate, 0, temp.length(), temp);
		}
		catch (IOException e)
		{
			System.err.println(e);
		}
	}

	//Call this method to get a file from the server
	void requestRemoteFile(String text, String localPath)
	{
		try
		{
//System.out.println("requestRemoteFile text: " + text);
//System.out.println("requestRemoteFile leng: " + text.length());
			String temp = text;
			receivePath = localPath;
					
			// 0 : compression not supported - 1 : compression supported
			writeRfbFileTransferMsg(rfbFileTransferRequest, 0, 1, temp.length(), temp);
		}
		catch (IOException e)
		{
			System.err.println(e);
		}
	}

	//Internally used when transferring file from server. Here, the server sends
	//a rfb packet signalling that it is ready to send the file requested
	void receiveFileHeader() throws IOException
	{
		fFileReceptionRunning = true;
		fFileReceptionError = false;
		viewer.ftp.disableButtons();
		int size = is.readInt();
		int length = is.readInt();

//System.out.println("receiveFileHeader size: " + size);
//System.out.println("receiveFileHeader leng: " + length);
		
		String tempName = "";
		for (int i = 0; i < length; i++)
		{
			tempName += (char) is.readUnsignedByte();
		}

		if (size == rfbRErrorCmd || size == -1) {
			fFileReceptionRunning = false;
			viewer.ftp.enableButtons();
			viewer.ftp.connectionStatus.setText("Remote file not available for reading.");
			viewer.ftp.historyComboBox.insertItemAt(new String(" > Error - Remote file not available for reading."), 0);
			viewer.ftp.historyComboBox.setSelectedIndex(0);
			return;
		}

		// sf@2004 - Read the high part of file size (not yet in rfbFileTransferMsg for 
		// backward compatibility reasons...)
		int sizeH = is.readInt();
		long lSize = ((long)(sizeH) << 32) + size;
		
		receiveFileSize = lSize;
		viewer.ftp.connectionStatus.setText("Received: 0 bytes of " + lSize + " bytes");
		fileSize=0;
		fileChunkCounter = 0;
		String fileName = receivePath;
		try {
			fos = new FileOutputStream(fileName);
		} catch (Exception e) {
			fFileReceptionRunning = false;
			writeRfbFileTransferMsg(rfbAbortFileTransfer, 0, 0, 0, null);
			viewer.ftp.historyComboBox.insertItemAt(new String(" > ERROR opening Local File: <" + fileName ),0);
			viewer.ftp.historyComboBox.setSelectedIndex(0);
			viewer.ftp.enableButtons();
			return;
		}
		writeRfbFileTransferMsg(rfbFileHeader, 0, 0, 0, null);
	}

	//Internally used when transferring file from server. This method receives one chunk
	//of the file
	void receiveFileChunk() throws IOException
	{
		// sf@2004 - Size = 0 means file chunck not compressed
		int size = is.readInt();
		boolean fCompressed = (size != 0);
		int length = is.readInt();
		fileChunkCounter++;

		// sf@2004 - allocates buffers for file chunck reception and decompression 
		byte[] ReceptionBuffer = new byte[length + 32];

		// Read the incoming file data
		// Todo: check error !
		is.readFully(ReceptionBuffer,0, length);
		
		if (fCompressed)
		{
			int bufSize = sz_rfbBlockSize + 1024; // Todo: set a more accurate value here
			int decompressedSize = 0;
			byte[] DecompressionBuffer = new byte[bufSize];
			Inflater myInflater = new Inflater();
			myInflater.setInput(ReceptionBuffer);
			try
			{
				decompressedSize = myInflater.inflate(DecompressionBuffer);
			}
			catch (DataFormatException e)
			{
				System.err.println(e);
			}
			// Todo: check error !
			fos.write(DecompressionBuffer, 0, decompressedSize);
			fileSize += decompressedSize;
		}
		else
		{
			//	 Todo: check error !
			fos.write(ReceptionBuffer, 0, length);
			fileSize += length;
		}
		
		/*
		for (int i = 0; i < length; i++) 
		{
			fos.write(is.readUnsignedByte());
			fileSize++;
		}
		*/
		
		// viewer.ftp.connectionStatus.setText("Received: " + fileSize + " bytes of "+ receiveFileSize+ " bytes" );
		viewer.ftp.jProgressBar.setValue((int)((fileSize * 100) / receiveFileSize));
		viewer.ftp.connectionStatus.setText(">>> Receiving File: " + receivePath + " - Size: " + receiveFileSize + " bytes - Progress: " + ((fileSize * 100) / receiveFileSize) + "%");
		
		if (fAbort == true)
		{
			fAbort = false;
			fFileReceptionError = true;
			writeRfbFileTransferMsg(rfbAbortFileTransfer, 0, 0, 0, null);

			//runge for use with x11vnc/libvncserver, no rfbAbortFileTransfer reply sent.
		        try {Thread.sleep(500);} catch (InterruptedException e) {}
			viewer.ftp.enableButtons();
			viewer.ftp.refreshLocalLocation();
			viewer.ftp.connectionStatus.setText(" > Error - File NOT received");
			viewer.ftp.historyComboBox.insertItemAt(new String(" > Error - File: <" + receivePath + "> not correctly received from Remote Machine (aborted by user or error)") ,0);
		}
		// sf@2004 - For old FT protocole only
		/*
		if(fileChunkCounter==10)
		{
			writeRfbFileTransferMsg(rfbFileHeader,0,0,0,null);
			fileChunkCounter=0;
		}
		*/
	}
	
	//Internally used when transferring file from server. Server signals end of file.
	void endOfReceiveFile(boolean fReceptionOk) throws IOException
	{
		int size = is.readInt();
		int length = is.readInt();
		fileSize=0;
		fos.close();

		viewer.ftp.refreshLocalLocation();
		if (fReceptionOk && !fFileReceptionError)
		{
			viewer.ftp.connectionStatus.setText(" > File successfully received");
			viewer.ftp.historyComboBox.insertItemAt(new String(" > File: <" + receivePath + "> received from Remote Machine" ),0);
		}
		else
		{
			// sf@2004 - Delete the incomplete receieved file for now (until we use Delta Transfer)
			File f = new File(receivePath);
			f.delete();		
			viewer.ftp.connectionStatus.setText(" > Error - File NOT received");
			viewer.ftp.historyComboBox.insertItemAt(new String(" > Error - File: <" + receivePath + "> not correctly received from Remote Machine (aborted by user or error)") ,0);
		}

		fFileReceptionError = false;
		fFileReceptionRunning = false;
		viewer.ftp.historyComboBox.setSelectedIndex(0);
		viewer.ftp.enableButtons();
	}

	//Call this method to read the contents of the server directory
	void readServerDirectory(String text)
	{
		try
		{
			String temp = text;
			writeRfbFileTransferMsg(rfbDirContentRequest, rfbRDirContent, 0, temp.length(), temp);
		}
		catch (IOException e)
		{
			System.err.println(e);
		}

	}

	//Internally used to receive list of drives available on the server
	void readFTPMsgDriveList() throws IOException
	{
		String str = "";
		for (int i = 0; i < 4; i++)
		{
			is.readUnsignedByte();
		}
		int length = is.readInt();
		for (int i = 0; i < length; i++)
		{
			char temp = (char) is.readUnsignedByte();
			if (temp != '\0')
			{
				str += temp;
			}
		}
		viewer.ftp.printDrives(str);
		
		// sf@2004
		// Finds the first readable drive and populates the local directory
		viewer.ftp.changeLocalDirectory(viewer.ftp.getFirstReadableLocalDrive());
		// Populate the remote directory
		viewer.ftp.changeRemoteDrive();
		viewer.ftp.refreshRemoteLocation();
		
	}

	//Internally used to receive directory content from server
	//Here, the server marks the start of the directory listing
	void readFTPMsgDirectoryList() throws IOException
	{
		is.readInt();
		int length = is.readInt();
		if (length == 0)
		{
			readFTPMsgDirectorydriveNotReady();
			inDirectory2 = false;
		}
		else
		{
			// sf@2004 - New FT protocole sends remote directory name
			String str = "";
			for (int i = 0; i < length; i++)
			{
				char temp = (char) is.readUnsignedByte();
				if (temp != '\0')
				{
					str += temp;
				}
			}
			// runge
			viewer.ftp.receivedRemoteDirectoryName(str);
			// viewer.ftp.changeRemoteDirectory(str);
			
		}
	}

	int zogswap(int n) {
		long l = n;
		if (l < 0) {
			l += 0x100000000L;
		}
		l = l & 0xFFFFFFFF;
		l = (l >> 24) | ((l & 0x00ff0000) >> 8) | ((l & 0x0000ff00) << 8) | (l << 24);
		return (int) l;
	}

	int windozeToUnix(int L, int H) {
		long L2 = zogswap(L);
		long H2 = zogswap(H);
		long unix = (H2 << 32) + L2;
		unix -= 11644473600L * 10000000L;
		unix /= 10000000L;
		//System.out.println("unix time: " + unix + " H2: " + H2 + " L2: " + L2);
		return (int) unix;
	}
	
	String timeStr(int t, int h) {
		if (h == 0) {
			// x11vnc/libvncserver unix
			t = zogswap(t);
		} else {
			// ultra (except if h==0 by chance)
			t = windozeToUnix(t, h);
		}
		long tl = (long) t;
		Date date = new Date(tl * 1000);
		if (true) {
			return date.toString();
		} else {
			return DateFormat.getDateTimeInstance().format(date);
		}
	}

	String dotPast(double f, int n) {
		String fs = "" + f;
		int i = fs.lastIndexOf(".") + n;
		if (i >= 0) {
			int len = fs.length();
			if (i >= len) {
				i = len-1;
			}
			fs = fs.substring(0, i);
		}
		return fs;
	}
	String sizeStr(int s) {
		s = zogswap(s);
		if (s < 0) {
			return s + "? B";
		} else if (s < 1024) {
			return s + " B";
		} else if (s < 1024 * 1024) {
			double k = s / 1024.0;
			String ks = dotPast(k, 3);
			
			return s + " (" + ks + " KB)";
		} else {
			double m = s / (1024.0*1024.0);
			String ms = dotPast(m, 3);
			return s + " (" + ms + " MB)";
		}
	}

	//Internally used to receive directory content from server
	//Here, the server sends one file/directory with it's attributes
	void readFTPMsgDirectoryListContent() throws IOException
	{
		String fileName = "", alternateFileName = "";
		byte contentType = 0;
		int contentParamT = 0;
		int contentParam = 0;
		byte temp = 0;
		int dwFileAttributes,
			nFileSizeHigh,
			nFileSizeLow,
			dwReserved0,
			dwReserved1;
		long ftCreationTime, ftLastAccessTime, ftLastWriteTime;
		int ftCreationTimeL, ftLastAccessTimeL, ftLastWriteTimeL;
		int ftCreationTimeH, ftLastAccessTimeH, ftLastWriteTimeH;
		char cFileName, cAlternateFileName;
		int length = 0;
		is.readInt();
		length = is.readInt();

		char[] chars = new char[4*length];
		int char_cnt = 0;
		for (int i = 0; i < chars.length; i++) {
			chars[i] = '\0';
		}
		
		dwFileAttributes = is.readInt();
		length -= 4;
		//ftCreationTime = is.readLong();
		ftCreationTimeL = is.readInt();
		ftCreationTimeH = is.readInt();
		length -= 8;
		//ftLastAccessTime = is.readLong();
		ftLastAccessTimeL = is.readInt();
		ftLastAccessTimeH = is.readInt();
		length -= 8;
		//ftLastWriteTime = is.readLong();
		ftLastWriteTimeL = is.readInt();
		ftLastWriteTimeH = is.readInt();
		length -= 8;
		nFileSizeHigh = is.readInt();
		length -= 4;
		nFileSizeLow = is.readInt();
		length -= 4;
		dwReserved0 = is.readInt();
		length -= 4;
		dwReserved1 = is.readInt();
		length -= 4;
		cFileName = (char) is.readUnsignedByte();
		length--;
		chars[char_cnt++] = cFileName;
		while (cFileName != '\0')
		{
			fileName += cFileName;
			cFileName = (char) is.readUnsignedByte();
			chars[char_cnt++] = cFileName;
			length--;
		}
		cAlternateFileName = (char) is.readByte();
		length--;
		while (length != 0)
		{
			alternateFileName += cAlternateFileName;
			cAlternateFileName = (char) is.readUnsignedByte();
			length--;
		}
		String guessed = guess_encoding(chars);
		if (!guessed.equals("ASCII")) {
			System.out.println("guess: " + guessed + "\t" + fileName);
		}
		if (guessed.equals("UTF-8")) {
			try {
				byte[] bytes = new byte[char_cnt-1];
				for (int i=0; i < char_cnt-1; i++) {
					bytes[i] = (byte) chars[i];
				}
				String newstr = new String(bytes, "UTF-8");
				fileName = newstr;
			} catch (Exception e) {
				System.out.println("failed to convert bytes to UTF-8 based string");
			}
		}
		for (int i = 0; i < char_cnt; i++) {
			//System.out.println("char[" + i + "]\t" + (int) chars[i]);
		}
		if (fileName.length() <= 0) {
			;
		} else if (dwFileAttributes == 268435456
			|| dwFileAttributes == 369098752
			|| dwFileAttributes == 285212672 
			|| dwFileAttributes == 271056896
			|| dwFileAttributes == 824705024
			||	dwFileAttributes == 807927808
			|| dwFileAttributes == 371720192
			|| dwFileAttributes == 369623040)
		{
			fileName = " [" + fileName + "]";
// begin runge/x11vnc
//			remoteDirsList.add(fileName); // sf@2004
			int i = -1;
			String t1 = fileName.toLowerCase();
			for (int j = 0; j < remoteDirsList.size(); j++) {
				String t = (String) remoteDirsList.get(j);
				String t2 = t.toLowerCase();
				if (t1.compareTo(t2) < 0) {
					i = j;
					break;
				}
			}
			//String s = "Lastmod: " + timeStr(ftLastWriteTimeL, ftLastWriteTimeH) + "    " + fileName; 
			String f2 = fileName;
			if (f2.length() < 24) {
				for (int ik = f2.length(); ik < 24; ik++) {
					f2 = f2 + " ";
				}
			}
			String s = f2 + "    \tLastmod: " + timeStr(ftLastWriteTimeL, ftLastWriteTimeH) + "    \t\tSize: " + sizeStr(nFileSizeLow); 
			//s = fileName + " Lastmod: " + zogswap(ftLastWriteTimeL); 
			if (i >= 0) {
				remoteDirsList.add(i, fileName);
				remoteDirsListInfo.add(i, s);
			} else {
				remoteDirsList.add(fileName);
				remoteDirsListInfo.add(s);
			}
// end runge/x11vnc
		} else {
// begin runge/x11vnc
//			remoteFilesList.add(" " + fileName); // sf@2004
	
			fileName = " " + fileName;
			int i = -1;
			String t1 = fileName.toLowerCase();
			for (int j = 0; j < remoteFilesList.size(); j++) {
				String t = (String) remoteFilesList.get(j);
				String t2 = t.toLowerCase();
				if (t1.compareTo(t2) < 0) {
					i = j;
					break;
				}
			}
			String f2 = fileName;
			if (f2.length() < 24) {
				for (int ik = f2.length(); ik < 24; ik++) {
					f2 = f2 + " ";
				}
			}

if (false) {
System.out.println("fileName:         " + f2);
System.out.println("ftLastWriteTimeL: " + ftLastWriteTimeL);
System.out.println("ftLastWriteTimeH: " + ftLastWriteTimeH);
System.out.println("nFileSizeLow:     " + nFileSizeLow);
}

			String s = f2 + "    \tLastmod: " + timeStr(ftLastWriteTimeL, ftLastWriteTimeH) + "    \t\tSize: " + sizeStr(nFileSizeLow); 
			//s = fileName + " Lastmod: " + ftLastWriteTimeL + "/" + zogswap(ftLastWriteTimeL) + "  Size: " + nFileSizeLow + "/" + zogswap(nFileSizeLow); 
			if (i >= 0) {
				remoteFilesList.add(i, fileName);
				remoteFilesListInfo.add(i, s);
			} else {
				remoteFilesList.add(fileName);
				remoteFilesListInfo.add(s);
			}
// end runge/x11vnc
		}
	
		// a.add(fileName);
	}

	//Internally used to read directory content of server.
	//Here, server signals end of directory.
	void readFTPMsgDirectoryListEndContent() throws IOException
	{
		is.readInt();
		int length = is.readInt();

		// sf@2004
		a.clear();
		b.clear();
		for (int i = 0; i < remoteDirsList.size(); i++) {
			a.add(remoteDirsList.get(i));
			b.add(remoteDirsListInfo.get(i));
		}
		for (int i = 0; i < remoteFilesList.size(); i++) {
			a.add(remoteFilesList.get(i));

			b.add(remoteFilesListInfo.get(i));
		}
		remoteDirsList.clear();
		remoteDirsListInfo.clear();
		remoteFilesList.clear();
		remoteFilesListInfo.clear();
		
// begin runge/x11vnc
		// Hack for double listing at startup... probably libvncserver bug..
		readServerDriveListCnt++;
		if (readServerDriveListCnt == 2) {
			if (System.currentTimeMillis() - readServerDriveListTime < 2000)  {
//System.out.println("readServerDriveListCnt skip " + readServerDriveListCnt);
				return;
			}
		}
// end runge/x11vnc
		viewer.ftp.printDirectory(a, b);
	}

	//Internally used to signify the drive requested is not ready

	void readFTPMsgDirectorydriveNotReady() throws IOException
	{
		System.out.println("Remote Drive unavailable");
		viewer.ftp.connectionStatus.setText(" > WARNING - Remote Drive unavailable (possibly restricted access or media not present)");
		viewer.ftp.remoteStatus.setText("WARNING: Remote Drive unavailable");
		viewer.ftp.historyComboBox.insertItemAt(new String(" > WARNING: Remote Drive unavailable."), 0);
		viewer.ftp.historyComboBox.setSelectedIndex(0);
	}

	//Call this method to request the list of drives on the server.
	void readServerDriveList()
	{
		try
		{
			viewer.rfb.writeRfbFileTransferMsg(RfbProto.rfbDirContentRequest, RfbProto.rfbRDrivesList, 0, 0, null);
// begin runge/x11vnc
			readServerDriveListCnt = 0;
			readServerDriveListTime = System.currentTimeMillis();
// end   runge/x11vnc
		}
		catch (IOException e)
		{
			System.err.println(e);
		}
	}

	// sf@2004 - Read the destination file checksums data
	// We don't use it for now
	void ReceiveDestinationFileChecksums() throws IOException
	{
		int size = is.readInt();
		int length = is.readInt();
		
		byte[] ReceptionBuffer = new byte[length + 32];

		// Read the incoming file data
		is.readFully(ReceptionBuffer,0, length);

		/*
		String csData = "";
		for (int i = 0; i < length; i++)
		{
			csData += (char) is.readUnsignedByte();
		}
		*/
	
		// viewer.ftp.connectionStatus.setText("Received: 0 bytes of " + size + " bytes");
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//
	// Write a FramebufferUpdateRequest message
	//

	void writeFramebufferUpdateRequest(
		int x,
		int y,
		int w,
		int h,
		boolean incremental)
		throws IOException {
		if (!viewer.ftp.isVisible()) {
			byte[] b = new byte[10];

			b[0] = (byte) FramebufferUpdateRequest;
			b[1] = (byte) (incremental ? 1 : 0);
			b[2] = (byte) ((x >> 8) & 0xff);
			b[3] = (byte) (x & 0xff);
			b[4] = (byte) ((y >> 8) & 0xff);
			b[5] = (byte) (y & 0xff);
			b[6] = (byte) ((w >> 8) & 0xff);
			b[7] = (byte) (w & 0xff);
			b[8] = (byte) ((h >> 8) & 0xff);
			b[9] = (byte) (h & 0xff);

			os.write(b);
		}
	}

	//
	// Write a SetPixelFormat message
	//

	void writeSetPixelFormat(
		int bitsPerPixel,
		int depth,
		boolean bigEndian,
		boolean trueColour,
		int redMax,
		int greenMax,
		int blueMax,
		int redShift,
		int greenShift,
		int blueShift,
		boolean fGreyScale) // sf@2005
		throws IOException {
		byte[] b = new byte[20];

		b[0] = (byte) SetPixelFormat;
		b[4] = (byte) bitsPerPixel;
		b[5] = (byte) depth;
		b[6] = (byte) (bigEndian ? 1 : 0);
		b[7] = (byte) (trueColour ? 1 : 0);
		b[8] = (byte) ((redMax >> 8) & 0xff);
		b[9] = (byte) (redMax & 0xff);
		b[10] = (byte) ((greenMax >> 8) & 0xff);
		b[11] = (byte) (greenMax & 0xff);
		b[12] = (byte) ((blueMax >> 8) & 0xff);
		b[13] = (byte) (blueMax & 0xff);
		b[14] = (byte) redShift;
		b[15] = (byte) greenShift;
		b[16] = (byte) blueShift;
		b[17] = (byte) (fGreyScale ? 1 : 0); // sf@2005

		os.write(b);

	}

	//
	// Write a FixColourMapEntries message.  The values in the red, green and
	// blue arrays are from 0 to 65535.
	//

	void writeFixColourMapEntries(
		int firstColour,
		int nColours,
		int[] red,
		int[] green,
		int[] blue)
		throws IOException {
		byte[] b = new byte[6 + nColours * 6];

		b[0] = (byte) FixColourMapEntries;
		b[2] = (byte) ((firstColour >> 8) & 0xff);
		b[3] = (byte) (firstColour & 0xff);
		b[4] = (byte) ((nColours >> 8) & 0xff);
		b[5] = (byte) (nColours & 0xff);

		for (int i = 0; i < nColours; i++) {
			b[6 + i * 6] = (byte) ((red[i] >> 8) & 0xff);
			b[6 + i * 6 + 1] = (byte) (red[i] & 0xff);
			b[6 + i * 6 + 2] = (byte) ((green[i] >> 8) & 0xff);
			b[6 + i * 6 + 3] = (byte) (green[i] & 0xff);
			b[6 + i * 6 + 4] = (byte) ((blue[i] >> 8) & 0xff);
			b[6 + i * 6 + 5] = (byte) (blue[i] & 0xff);
		}

		os.write(b);

	}

	//
	// Write a SetEncodings message
	//

	void writeSetEncodings(int[] encs, int len) throws IOException {
		byte[] b = new byte[4 + 4 * len];

		b[0] = (byte) SetEncodings;
		b[2] = (byte) ((len >> 8) & 0xff);
		b[3] = (byte) (len & 0xff);

		for (int i = 0; i < len; i++) {
			b[4 + 4 * i] = (byte) ((encs[i] >> 24) & 0xff);
			b[5 + 4 * i] = (byte) ((encs[i] >> 16) & 0xff);
			b[6 + 4 * i] = (byte) ((encs[i] >> 8) & 0xff);
			b[7 + 4 * i] = (byte) (encs[i] & 0xff);
		}

		os.write(b);

	}

	//
	// Write a ClientCutText message
	//

	void writeClientCutText(String text) throws IOException {
	//	if (!viewer.ftp.isVisible()) {

		byte[] b = new byte[8 + text.length()];

		b[0] = (byte) ClientCutText;
		b[4] = (byte) ((text.length() >> 24) & 0xff);
		b[5] = (byte) ((text.length() >> 16) & 0xff);
		b[6] = (byte) ((text.length() >> 8) & 0xff);
		b[7] = (byte) (text.length() & 0xff);

		if (false && max_char(text) > 255) {
			System.arraycopy(text.getBytes("UTF-8"), 0, b, 8, text.length());
		} else if (max_char(text) > 127) {
			System.arraycopy(text.getBytes("ISO-8859-1"), 0, b, 8, text.length());
		} else {
			System.arraycopy(text.getBytes(), 0, b, 8, text.length());
		}

		os.write(b);
	//	}
	}

	//
	// A buffer for putting pointer and keyboard events before being sent.  This
	// is to ensure that multiple RFB events generated from a single Java Event 
	// will all be sent in a single network packet.  The maximum possible
	// length is 4 modifier down events, a single key event followed by 4
	// modifier up events i.e. 9 key events or 72 bytes.
	//

	byte[] eventBuf = new byte[72];
	int eventBufLen;

	// Useful shortcuts for modifier masks.

	final static int CTRL_MASK = InputEvent.CTRL_MASK;
	final static int SHIFT_MASK = InputEvent.SHIFT_MASK;
	final static int META_MASK = InputEvent.META_MASK;
	final static int ALT_MASK = InputEvent.ALT_MASK;

	void writeWheelEvent(MouseWheelEvent evt) throws IOException {
		eventBufLen = 0;

		int x = evt.getX();
		int y = evt.getY();

		if (x < 0) x = 0;
		if (y < 0) y = 0;

		int ptrmask;

		int clicks = evt.getWheelRotation();
		System.out.println("writeWheelEvent: clicks: " + clicks);
		if (clicks > 0) {
			ptrmask = 16;
		} else if (clicks < 0) {
			ptrmask = 8;
		} else {
			return;
		}

		eventBuf[eventBufLen++] = (byte) PointerEvent;
		eventBuf[eventBufLen++] = (byte) ptrmask;
		eventBuf[eventBufLen++] = (byte) ((x >> 8) & 0xff);
		eventBuf[eventBufLen++] = (byte) (x & 0xff);
		eventBuf[eventBufLen++] = (byte) ((y >> 8) & 0xff);
		eventBuf[eventBufLen++] = (byte) (y & 0xff);

		os.write(eventBuf, 0, eventBufLen);
	}

	//
	// Write a pointer event message.  We may need to send modifier key events
	// around it to set the correct modifier state.
	//

	int pointerMask = 0;

	void writePointerEvent(MouseEvent evt) throws IOException {
		if (!viewer.ftp.isVisible()) {
		int modifiers = evt.getModifiers();

		int mask2 = 2;
		int mask3 = 4;
		if (viewer.options.reverseMouseButtons2And3) {
			mask2 = 4;
			mask3 = 2;
		}

		// Note: For some reason, AWT does not set BUTTON1_MASK on left
		// button presses. Here we think that it was the left button if
		// modifiers do not include BUTTON2_MASK or BUTTON3_MASK.

		if (evt.getID() == MouseEvent.MOUSE_PRESSED) {
			if ((modifiers & InputEvent.BUTTON2_MASK) != 0) {
				pointerMask = mask2;
				modifiers &= ~ALT_MASK;
			} else if ((modifiers & InputEvent.BUTTON3_MASK) != 0) {
				pointerMask = mask3;
				modifiers &= ~META_MASK;
			} else {
				pointerMask = 1;
			}
		} else if (evt.getID() == MouseEvent.MOUSE_RELEASED) {
			pointerMask = 0;
			if ((modifiers & InputEvent.BUTTON2_MASK) != 0) {
				modifiers &= ~ALT_MASK;
			} else if ((modifiers & InputEvent.BUTTON3_MASK) != 0) {
				modifiers &= ~META_MASK;
			}
		}

		eventBufLen = 0;
		writeModifierKeyEvents(modifiers);

		int x = evt.getX();
		int y = evt.getY();

		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;

		eventBuf[eventBufLen++] = (byte) PointerEvent;
		eventBuf[eventBufLen++] = (byte) pointerMask;
		eventBuf[eventBufLen++] = (byte) ((x >> 8) & 0xff);
		eventBuf[eventBufLen++] = (byte) (x & 0xff);
		eventBuf[eventBufLen++] = (byte) ((y >> 8) & 0xff);
		eventBuf[eventBufLen++] = (byte) (y & 0xff);

		//
		// Always release all modifiers after an "up" event
		//

		if (pointerMask == 0) {
			writeModifierKeyEvents(0);
		}

		os.write(eventBuf, 0, eventBufLen);
		}
	}

	//
	// Write a key event message.  We may need to send modifier key events
	// around it to set the correct modifier state.  Also we need to translate
	// from the Java key values to the X keysym values used by the RFB protocol.
	//

	void writeKeyEvent(KeyEvent evt) throws IOException {
		if (!viewer.ftp.isVisible()) {
		int keyChar = evt.getKeyChar();

		//
		// Ignore event if only modifiers were pressed.
		//

		// Some JVMs return 0 instead of CHAR_UNDEFINED in getKeyChar().
		if (keyChar == 0)
			keyChar = KeyEvent.CHAR_UNDEFINED;

		if (keyChar == KeyEvent.CHAR_UNDEFINED) {
			int code = evt.getKeyCode();
			if (code == KeyEvent.VK_CONTROL
				|| code == KeyEvent.VK_SHIFT
				|| code == KeyEvent.VK_META
				|| code == KeyEvent.VK_ALT)
				return;
		}

		//
		// Key press or key release?
		//

		boolean down = (evt.getID() == KeyEvent.KEY_PRESSED);

		if (viewer.debugKeyboard) {
			System.out.println("----------------------------------------");
			System.out.println("evt.getKeyChar:      " + evt.getKeyChar());
			System.out.println("getKeyText:          " + KeyEvent.getKeyText(evt.getKeyCode()));
			System.out.println("evt.getKeyCode:      " + evt.getKeyCode());
			System.out.println("evt.getID:           " + evt.getID());
			System.out.println("evt.getKeyLocation:  " + evt.getKeyLocation());
			System.out.println("evt.isActionKey:     " + evt.isActionKey());
			System.out.println("evt.isControlDown:   " + evt.isControlDown());
			System.out.println("evt.getModifiers:    " + evt.getModifiers());
			System.out.println("getKeyModifiersText: " + KeyEvent.getKeyModifiersText(evt.getModifiers()));
			System.out.println("evt.paramString:     " + evt.paramString());
		}


		int key;
		if (evt.isActionKey()) {

			//
			// An action key should be one of the following.
			// If not then just ignore the event.
			//

			switch (evt.getKeyCode()) {
				case KeyEvent.VK_HOME :
					key = 0xff50;
					break;
				case KeyEvent.VK_LEFT :
					key = 0xff51;
					break;
				case KeyEvent.VK_UP :
					key = 0xff52;
					break;
				case KeyEvent.VK_RIGHT :
					key = 0xff53;
					break;
				case KeyEvent.VK_DOWN :
					key = 0xff54;
					break;
				case KeyEvent.VK_PAGE_UP :
					key = 0xff55;
					break;
				case KeyEvent.VK_PAGE_DOWN :
					key = 0xff56;
					break;
				case KeyEvent.VK_END :
					key = 0xff57;
					break;
				case KeyEvent.VK_INSERT :
					key = 0xff63;
					break;
				case KeyEvent.VK_F1 :
					key = 0xffbe;
					break;
				case KeyEvent.VK_F2 :
					key = 0xffbf;
					break;
				case KeyEvent.VK_F3 :
					key = 0xffc0;
					break;
				case KeyEvent.VK_F4 :
					key = 0xffc1;
					break;
				case KeyEvent.VK_F5 :
					key = 0xffc2;
					break;
				case KeyEvent.VK_F6 :
					key = 0xffc3;
					break;
				case KeyEvent.VK_F7 :
					key = 0xffc4;
					break;
				case KeyEvent.VK_F8 :
					key = 0xffc5;
					break;
				case KeyEvent.VK_F9 :
					key = 0xffc6;
					break;
				case KeyEvent.VK_F10 :
					key = 0xffc7;
					break;
				case KeyEvent.VK_F11 :
					key = 0xffc8;
					break;
				case KeyEvent.VK_F12 :
					key = 0xffc9;
					break;
				default :
					return;
			}
			if (key == 0xffc2 && viewer.mapF5_to_atsign) {
				key = 0x40;
			}

		} else {

			//
			// A "normal" key press.  Ordinary ASCII characters go straight through.
			// For CTRL-<letter>, CTRL is sent separately so just send <letter>.
			// Backspace, tab, return, escape and delete have special keysyms.
			// Anything else we ignore.
			//

			key = keyChar;

			if (key < 0x20) {
				if (evt.isControlDown()) {
					key += 0x60;
				} else {
					switch (key) {
						case KeyEvent.VK_BACK_SPACE :
							key = 0xff08;
							break;
						case KeyEvent.VK_TAB :
							key = 0xff09;
							break;
						case KeyEvent.VK_ENTER :
							key = 0xff0d;
							break;
						case KeyEvent.VK_ESCAPE :
							key = 0xff1b;
							break;
					}
				}
			} else if (key == 0x7f) {
				// Delete
				key = 0xffff;
			} else if (key > 0xff) {
				// JDK1.1 on X incorrectly passes some keysyms straight through,
				// so we do too.  JDK1.1.4 seems to have fixed this.
				// The keysyms passed are 0xff00 .. XK_BackSpace .. XK_Delete
				if ((key < 0xff00) || (key > 0xffff))
					return;
			}
		}

		// Fake keyPresses for keys that only generates keyRelease events
		if ((key == 0xe5)
			|| (key == 0xc5)
			|| // XK_aring / XK_Aring
		 (key == 0xe4)
			|| (key == 0xc4)
			|| // XK_adiaeresis / XK_Adiaeresis
		 (key == 0xf6)
			|| (key == 0xd6)
			|| // XK_odiaeresis / XK_Odiaeresis
		 (key == 0xa7)
			|| (key == 0xbd)
			|| // XK_section / XK_onehalf
		 (key == 0xa3)) { // XK_sterling
			// Make sure we do not send keypress events twice on platforms
			// with correct JVMs (those that actually report KeyPress for all
			// keys)	
			if (down)
				brokenKeyPressed = true;

			if (!down && !brokenKeyPressed) {
				// We've got a release event for this key, but haven't received
				// a press. Fake it. 
				eventBufLen = 0;
				writeModifierKeyEvents(evt.getModifiers());
				writeKeyEvent(key, true);
				os.write(eventBuf, 0, eventBufLen);
			}

			if (!down)
				brokenKeyPressed = false;
		}

		eventBufLen = 0;
		writeModifierKeyEvents(evt.getModifiers());
		writeKeyEvent(key, down);

		// Always release all modifiers after an "up" event
		if (!down)
			writeModifierKeyEvents(0);

		os.write(eventBuf, 0, eventBufLen);
		}
	}
	//
	// Add a raw key event with the given X keysym to eventBuf.
	//

	void writeKeyEvent(int keysym, boolean down) {
		eventBuf[eventBufLen++] = (byte) KeyboardEvent;
		eventBuf[eventBufLen++] = (byte) (down ? 1 : 0);
		eventBuf[eventBufLen++] = (byte) 0;
		eventBuf[eventBufLen++] = (byte) 0;
		eventBuf[eventBufLen++] = (byte) ((keysym >> 24) & 0xff);
		eventBuf[eventBufLen++] = (byte) ((keysym >> 16) & 0xff);
		eventBuf[eventBufLen++] = (byte) ((keysym >> 8) & 0xff);
		eventBuf[eventBufLen++] = (byte) (keysym & 0xff);
	}

	//
	// Write key events to set the correct modifier state.
	//

	int oldModifiers = 0;

	void writeModifierKeyEvents(int newModifiers) {
		if(viewer.forbid_Ctrl_Alt) {
			if ((newModifiers & CTRL_MASK) != 0 && (newModifiers & ALT_MASK) != 0) {
				int orig = newModifiers;
				newModifiers &= ~ALT_MASK;
				newModifiers &= ~CTRL_MASK;
				if (viewer.debugKeyboard) {
					System.out.println("Ctrl+Alt modifiers: " + orig + " -> " + newModifiers);
				}
			}
		}
		if ((newModifiers & CTRL_MASK) != (oldModifiers & CTRL_MASK))
			writeKeyEvent(0xffe3, (newModifiers & CTRL_MASK) != 0);

		if ((newModifiers & SHIFT_MASK) != (oldModifiers & SHIFT_MASK))
			writeKeyEvent(0xffe1, (newModifiers & SHIFT_MASK) != 0);

		if ((newModifiers & META_MASK) != (oldModifiers & META_MASK))
			writeKeyEvent(0xffe7, (newModifiers & META_MASK) != 0);

		if ((newModifiers & ALT_MASK) != (oldModifiers & ALT_MASK))
			writeKeyEvent(0xffe9, (newModifiers & ALT_MASK) != 0);

		oldModifiers = newModifiers;
	}

	//
	// Compress and write the data into the recorded session file. This
	// method assumes the recording is on (rec != null).
	//

	void recordCompressedData(byte[] data, int off, int len)
		throws IOException {
		Deflater deflater = new Deflater();
		deflater.setInput(data, off, len);
		int bufSize = len + len / 100 + 12;
		byte[] buf = new byte[bufSize];
		deflater.finish();
		int compressedSize = deflater.deflate(buf);
		recordCompactLen(compressedSize);
		rec.write(buf, 0, compressedSize);
	}

	void recordCompressedData(byte[] data) throws IOException {
		recordCompressedData(data, 0, data.length);
	}

	//
	// Write an integer in compact representation (1..3 bytes) into the
	// recorded session file. This method assumes the recording is on
	// (rec != null).
	//

	void recordCompactLen(int len) throws IOException {
		byte[] buf = new byte[3];
		int bytes = 0;
		buf[bytes++] = (byte) (len & 0x7F);
		if (len > 0x7F) {
			buf[bytes - 1] |= 0x80;
			buf[bytes++] = (byte) (len >> 7 & 0x7F);
			if (len > 0x3FFF) {
				buf[bytes - 1] |= 0x80;
				buf[bytes++] = (byte) (len >> 14 & 0xFF);
			}
		}
		rec.write(buf, 0, bytes);
	}
}
