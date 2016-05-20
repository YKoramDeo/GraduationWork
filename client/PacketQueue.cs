using System;
using System.Collections.Generic;
using System.IO;

public class PacketQueue
{	
	// 패킷 저장 정보.
	struct PacketInfo
	{
		public int	offset;
		public int 	size;
	};
	
	// 데이터를 보존할 버퍼
	private MemoryStream 		mStreamBuffer;
	// 패킷 정보 관리 리스트
	private List<PacketInfo>	mOffsetList;
	// 메모리 배치 오프셋
	private int					mOffset = 0;

    // Lock
    private Object              mLock;

	// 생성자(초기화)
	public PacketQueue()
	{
		mStreamBuffer = new MemoryStream();
		mOffsetList = new List<PacketInfo>();
        mLock = new object();
	}
	
	public int Enqueue(byte[] data, int size)
	{
		PacketInfo	info = new PacketInfo();
	    
        lock (mLock)
        {
            info.offset = mOffset;
            info.size = size;

            // 패킷 저장 정보 보존.
            mOffsetList.Add(info);

            // 패킷 데이터를 보존.
            mStreamBuffer.Position = mOffset;
            mStreamBuffer.Write(data, 0, size);
            mStreamBuffer.Flush();
            mOffset += size;
        }
		return size;
	}
	
	public int Dequeue(ref byte[] buffer, int size) {

		if (mOffsetList.Count <= 0) {
			return -1;
		}

        int recvSize;

		lock(mLock)
        {
            PacketInfo info = mOffsetList[0];

            // 버퍼로부터 해당하는 패킷 데이터를 가져옵니다.
            int dataSize = Math.Min(size, info.size);
            mStreamBuffer.Position = info.offset;
            recvSize = mStreamBuffer.Read(buffer, 0, dataSize);

            // 큐 데이터를 추출했으므로 선두 요소를 삭제합니다.
            if (recvSize > 0)
            {
                mOffsetList.RemoveAt(0);
            }

            // 모든 큐 데이터를 추출했으면 스트림을 클리어해서 메모리를 절약합니다.
            if (mOffsetList.Count == 0)
            {
                Clear();
                mOffset = 0;
            }
        }

		return recvSize;
	}
	
	public void Clear()
	{
		byte[] buffer = mStreamBuffer.GetBuffer();
		Array.Clear(buffer, 0, buffer.Length);
		
		mStreamBuffer.Position = 0;
		mStreamBuffer.SetLength(0);
	}
}