using UnityEngine;
using System.Net.Sockets;
using System.Threading;

public class NetworkLib
{
    private Socket mSocket;
    private PacketQueue mSendQueue;
    private PacketQueue mRecvQueue;

    private bool mConnecting;

    private Thread mDispatchSendThread;
    private Thread mDispatchRecvThread;

    public NetworkLib()
    {
        mSocket = null;
        mSendQueue = new PacketQueue();
        mRecvQueue = new PacketQueue();

        mDispatchSendThread = null;
        mDispatchRecvThread = null;

        mConnecting = false;
    }

    public bool Connect(string address, int port)
    {
        try
        {
            mSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            mSocket.NoDelay = true;
            mSocket.Connect(address, port);
            mSocket.SendBufferSize = 0;
        }
        catch
        {
            Debug.Log("Network::Connect fail!");
            return false;
        }

        LaunchThread();
        Debug.Log("Connect :: Called!!");
        return true;
    }

    public bool Disconnect()
    {
        try
        {
            if (null != mSocket)
            {
                mSocket.Shutdown(SocketShutdown.Both);
                // Socket 객체가 데이터를 전송하고 수신하는 기능을 비활성화한다.
                // 파라미터는 Send, Receive, Both가 있다.
                mSocket.Close();
            }
            mConnecting = false;
        }
        catch
        {
            Debug.Log("Network::Disonnect fail!");
            return false;
        }
        mDispatchSendThread.Join();
        mDispatchRecvThread.Join();
        return true;
    }

    // thread start function
    private void LaunchThread()
    {
        try
        {
            // accept thread start
            mConnecting = true;

            mDispatchSendThread = new Thread(new ThreadStart(DispatchSend));
            mDispatchRecvThread = new Thread(new ThreadStart(DispatchReceive));

            mDispatchSendThread.Start();
            mDispatchRecvThread.Start();

        }
        catch
        {
            Debug.Log("Network::Cannot launch thread.");
        }

        return;
    }

    public void DispatchSend()
    {
        while (mConnecting)
        {
            byte[] buffer = new byte[(int)NetConfig.MAX_BUFFER_SIZE];

            int nSendSize = mSendQueue.Dequeue(ref buffer, buffer.Length);

            while (nSendSize > 0)
            {
                mSocket.Send(buffer, nSendSize, SocketFlags.None);
                nSendSize = mSendQueue.Dequeue(ref buffer, buffer.Length);
            }
        }
        return;
    }

    private void DispatchReceive()
    {
        while (mConnecting)
        {
            if (mSocket.Poll(0, SelectMode.SelectRead))
            {
                byte[] buffer = new byte[(int)NetConfig.MAX_BUFFER_SIZE];
                int nRecvSize = mSocket.Receive(buffer, buffer.Length, SocketFlags.None);
                if (nRecvSize == 0)
                {
                    // disconnect and exit
                    Disconnect();
                }
                else if (nRecvSize > 0)
                {
                    mRecvQueue.Enqueue(buffer, nRecvSize);
                }
            }
        }
        return;
    }

    public int Send(byte[] data, int size)
    {
        return mSendQueue.Enqueue(data, size);
    }

    public int Receive(ref byte[] buffer, int size)
    {
        return mRecvQueue.Dequeue(ref buffer, size);
    }
}