using UnityEngine;
using System.Collections;

public class PlayerMovement : MonoBehaviour
{
    public AudioClip shoutingClip;      // Audio clip of the player shouting.
    public float turnSmoothing = 15f;   // A smoothing value for turning the player.
    public float speedDampTime = 0.1f;  // The damping for the speed parameter
    public Transform m_Cam;                  // A reference to the main camera in the scenes transform
    private Vector3 m_CamForward;             // The current forward direction of the camera
    private Vector3 m_Move;

    private Animator anim;              // Reference to the animator component.
    private HashIDs hash;               // Reference to the HashIDs.
    private Transform enemy;
    private LastPlayerSighting lastPlayerSight;
    private EnemySight enemySight;
    private FlashLight m_flashLight;
    private MobileNetworkControll mobileController;
    private GameObject controllerCamera;
    private GameObject flashLightVec;
    private NavMeshAgent nav;
    private int i = 0;

    public float mobile_look_h;
    public float mobile_look_v;
    public bool mobile_look_btn;
    public bool mobile_look_shout;

    //*************************************Start*************************************
    private NetworkMgr mNetworkMgr;
    private int mFrameCount;
    public int mInstanceID;
    float h, v;
    bool sneak;
    bool flashLight;
    bool shout;

    NavMeshPath path;

    private Vector3 otherPlayerPos;
    private Vector3 pre_otherPlayerPos;

    public Quaternion sendFlashLightrotation = Quaternion.identity;
    //private GameObject otherFlashLight;
    //**************************************End**************************************

    void Awake()
    {
        // Setting up the references.
        anim = GetComponent<Animator>();

        enemy = GameObject.FindGameObjectWithTag(Tags.enemy).GetComponent<Transform>();

        enemySight = GameObject.FindGameObjectWithTag(Tags.enemy).GetComponent<EnemySight>();
        //m_flashLight = GameObject.FindGameObjectWithTag(Tags.flashLight).GetComponent<FlashLight>();
        m_flashLight = transform.Find("FlashLight").gameObject.GetComponent<FlashLight>();
        // Set the weight of the shouting layer to 1.
        anim.SetLayerWeight(1, 1f);

        controllerCamera = GameObject.FindGameObjectWithTag(Tags.controllCamera);
        flashLightVec = GameObject.FindGameObjectWithTag(Tags.flashLight);


        mInstanceID = (int)NetConfig.NIL;
        mFrameCount = 0;

        path = new NavMeshPath();

        otherPlayerPos = transform.position;
        pre_otherPlayerPos = Vector3.zero;

    }

    void Start()
    {
        mNetworkMgr = NetworkMgr.GetInstance();
        hash = GameObject.FindGameObjectWithTag(Tags.gameController).GetComponent<HashIDs>();
        lastPlayerSight = GameObject.FindGameObjectWithTag(Tags.gameController).GetComponent<LastPlayerSighting>();
        mobileController = GameObject.FindGameObjectWithTag("MobileController").GetComponent<MobileNetworkControll>();

        GameObject.FindGameObjectWithTag(Tags.player).GetComponent<PlayerMovement>().mInstanceID = mNetworkMgr.GetMyID();

        nav = GetComponent<NavMeshAgent>();
        if (mNetworkMgr.GetMyID() != mInstanceID)
        {
            nav.enabled = false;
        }

        mNetworkMgr.RegisterReceiveNotification(PacketType.PlayerMove, OnReceivePlayerMovePacket);
        mNetworkMgr.RegisterReceiveNotification(PacketType.PlayerLight, OnReceivePlayerLightPacket);

        if (mNetworkMgr.GetMyID() == mInstanceID)
        {
            StartCoroutine(SendPacketFunc());
        }
    }

    void FixedUpdate()
    {
        if (mNetworkMgr.GetMyID() == mInstanceID)
        {

            mFrameCount++;

            // Cache the inputs.
            //키보드 부분
            h = Input.GetAxis("Horizontal");
            v = Input.GetAxis("Vertical");
            sneak = Input.GetButton("Sneak");
            flashLight = Input.GetButton("FlashLight");
            m_Cam = controllerCamera.transform;

            //스마트폰 부분
            if (mobileController.m_bMobileController)
            {
                h = mobile_look_h;
                v = mobile_look_v;
                sneak = Input.GetButton("Sneak");
                flashLight = mobile_look_btn;

                //플레시손전등 비추는방향으로 이동
                m_Cam = flashLightVec.transform;

                if (-0.3f < h && h < 0.3f && -0.5f < v && v < 0.5f)
                    sneak = true;
                else
                    sneak = false;
            }


            if (flashLight)
                sendFlashLightrotation = flashLightVec.transform.rotation;

            m_CamForward = Vector3.Scale(m_Cam.forward, new Vector3(1, 0, 1)).normalized;
            m_Move = v * m_CamForward + h * m_Cam.right;

            MovementManagement(h, v, sneak, flashLight, m_Move);

        }

        else //Other Player의 동작
        {
            transform.FindChild("FlashLight").gameObject.transform.rotation = Quaternion.Euler(0, 0, 0) * sendFlashLightrotation;

            nav.enabled = true;
            //nav.speed = 0.2f;
            //Rotating();
            //nav.pat
            nav.destination = otherPlayerPos;
            OtherPlayerMovement();

            // Set the animator shouting parameter.
            anim.SetBool(hash.shoutingBool, shout);

            AudioManagement(shout);
        }
    }

    private IEnumerator SendPacketFunc()
    {
        int frame_count = 0;
        while (true)
        {
            if (frame_count % 10 == 0)
            {
                if (Input.GetKeyDown(KeyCode.W) || Input.GetKey(KeyCode.W)
                    || Input.GetKeyDown(KeyCode.S) || Input.GetKey(KeyCode.S)
                    || Input.GetKeyDown(KeyCode.A) || Input.GetKey(KeyCode.A)
                    || Input.GetKeyDown(KeyCode.D) || Input.GetKey(KeyCode.D))
                    mNetworkMgr.SendPlayerMovePacket(m_Move, h, v, sneak, transform.position);
                
                if(Input.GetMouseButtonDown(0) || Input.GetMouseButton(0) || Input.GetMouseButtonUp(0))
                    mNetworkMgr.SendPlayerLightPacket(flashLight, sendFlashLightrotation);
            }
            yield return null;
        }
    }

    private void OnReceivePlayerMovePacket(PacketType type, byte[] packetData)
    {
        PlayerMovePacket packet = new PlayerMovePacket(packetData);
        PlayerMoveData data = packet.GetPacketData();

        Vector3 dir = Vector3.zero;
        float horizental, vertical;
        bool boolean;
        PlayerInfo info = new PlayerInfo();

        info.id = data.id;
        info.pos.x = data.posX;
        info.pos.y = data.posY;
        info.pos.z = data.posZ;
        dir.x = data.dirX;
        dir.y = data.dirY;
        dir.z = data.dirZ;
        horizental = data.horizental;
        vertical = data.vertical;
        boolean = data.sneak;

        if (info.id == mInstanceID)
        {
            m_Move = dir;
            h = horizental;
            v = vertical;
            sneak = boolean;

            otherPlayerPos = info.pos;
        }

        Debug.Log("OnReceivePlayerMovePacket::Called!");

        return;
    }

    private void OnReceivePlayerLightPacket(PacketType type, byte[] packetData)
    {
        PlayerLightPacket packet = new PlayerLightPacket(packetData);
        PlayerLightData data = packet.GetPacketData();
        int id;
        bool on;
        Quaternion rotation;

        id = data.id;
        on = data.on;
        rotation.x = data.rotX;
        rotation.y = data.rotY;
        rotation.z = data.rotZ;
        rotation.w = data.rotW;

        if (id == mInstanceID)
        {
            flashLight = on;
            sendFlashLightrotation = rotation;
        }
        return;
    }

    private void OnReceivePlayerShoutPacket(byte[] packetData)
    {
        PlayerShoutPacket packet = new PlayerShoutPacket(packetData);
        PlayerShoutData data = packet.GetPacketData();

        int id;
        bool shoutBool;
        Vector3 otherPlayerPosition;

        id = data.id;
        shoutBool = data.shouting;
        otherPlayerPosition.x = data.posX;
        otherPlayerPosition.y = data.posY;
        otherPlayerPosition.z = data.posZ;

        if (id == mInstanceID)
        {
            shout = shoutBool;
        }

        return;
    }

    void Update()
    {
        if (mNetworkMgr.GetMyID() == mInstanceID)
        {
            // Cache the attention attracting input.
            //키보드 부분
            shout = Input.GetButtonDown("Attract");

            if (mobileController.m_bMobileController)
                shout = mobile_look_shout; //스마트폰 부분

            // Set the animator shouting parameter.
            anim.SetBool(hash.shoutingBool, shout);

            AudioManagement(shout);
        }
    }

    void OtherPlayerMovement()
    {
        float distance = Vector3.Distance(transform.position, otherPlayerPos);

        if ((int)transform.position.x != (int)otherPlayerPos.x && (int)transform.position.z != (int)otherPlayerPos.z)
        {
            if (pre_otherPlayerPos != otherPlayerPos)
            {
                pre_otherPlayerPos = otherPlayerPos;
                // Create a path and set it based on a target position.
                if (nav.enabled)
                    nav.CalculatePath(otherPlayerPos, path);
            }

            if (path.corners.Length != 0)
            {
                // Create an array of points which is the length of the number of corners in the path + 2.
                Vector3[] allWayPoints = new Vector3[path.corners.Length + 2];

                // The first point is the enemy's position.
                allWayPoints[0] = transform.position;

                // The last point is the target position.
                allWayPoints[allWayPoints.Length - 1] = otherPlayerPos;

                // The points inbetween are the corners of the path.
                for (int k = 0; k < path.corners.Length; k++)
                {
                    allWayPoints[k + 1] = path.corners[k];
                }

                // Create a float to store the path length that is by default 0.
                Vector3 otherDirection = Vector3.zero;

                if (i != allWayPoints.Length)
                {
                    otherDirection = (allWayPoints[i + 1] - allWayPoints[i]);

                    if ((i + 1) > allWayPoints.Length || (i + 1) == allWayPoints.Length)
                        Debug.Log("error");

                    if ((int)otherDirection.x != 0 && (int)otherDirection.y != 0 && (int)otherDirection.z != 0)
                        MovementManagement(1f, 1f, sneak, flashLight, otherDirection);
                }

                distance = Vector3.Distance(transform.position, allWayPoints[i + 1]);

                // Increment the path length by an amount equal to the distance between each waypoint and the next.
                if (distance < 0.5f)
                {
                    i++;
                    if (i == (allWayPoints.Length - 1))
                    {
                        MovementManagement(0, 0, sneak, flashLight, otherDirection);
                        i = 0;
                    }
                }
            }
            else
            {
                MovementManagement(h, v, sneak, flashLight, m_Move);
            }
        }
        else
        {
            i = 0;
            MovementManagement(h, v, sneak, flashLight, m_Move);
        }
    }

    void MovementManagement(float horizontal, float vertical, bool sneaking, bool flasLight, Vector3 moveDirection)
    {
        // Set the sneaking parameter to the sneak input.
        anim.SetBool(hash.sneakingBool, sneaking);

        if (flasLight)
        {
            m_flashLight.flashLightBool = true;
        }
        else
            m_flashLight.flashLightBool = false;

        // If there is some axis input...
        if (horizontal != 0f || vertical != 0f)
        {
            // ... set the players rotation and set the speed parameter to 5.5f.
            Rotating(moveDirection);
            anim.SetFloat(hash.speedFloat, 5.5f, speedDampTime, Time.deltaTime);
        }
        else
            // Otherwise set the speed parameter to 0.
            anim.SetFloat(hash.speedFloat, 0.0f, speedDampTime, Time.deltaTime);
    }


    void Rotating(Vector3 moveDirection)
    {
        // Create a new vector of the horizontal and vertical inputs.
        Vector3 targetDirection = moveDirection;

        // Create a rotation based on this new vector assuming that up is the global y axis.
        Quaternion targetRotation = Quaternion.LookRotation(targetDirection, Vector3.up);

        // Create a rotation that is an increment closer to the target rotation from the player's rotation.
        Quaternion newRotation = Quaternion.Lerp(GetComponent<Rigidbody>().rotation, targetRotation, turnSmoothing * Time.deltaTime);

        // Change the players rotation to this new rotation.
        GetComponent<Rigidbody>().MoveRotation(newRotation);
    }


    void AudioManagement(bool shout)
    {
        if (mNetworkMgr.GetMyID() == mInstanceID)
        {
            // If the player is currently in the run state...
            if (anim.GetCurrentAnimatorStateInfo(0).nameHash == hash.locomotionState)
            {
                // ... and if the footsteps are not playing...
                if (!GetComponent<AudioSource>().isPlaying)
                    // ... play them.
                    GetComponent<AudioSource>().Play();
            }
            else
                // Otherwise stop the footsteps.
                GetComponent<AudioSource>().Stop();

            // If the shout input has been pressed...
            if (shout)
            {
                // ... play the shouting clip where we are.
                AudioSource.PlayClipAtPoint(shoutingClip, transform.position);

                if (Vector3.Distance(enemy.position, transform.position) < 100.0f)
                {
                    enemySight.playerInSight = true;
                    lastPlayerSight.position = transform.position;
                }
            }
        }
        else
        {
            // If the player is currently in the run state...
            if (anim.GetCurrentAnimatorStateInfo(0).nameHash == hash.locomotionState)
            {
                // ... and if the footsteps are not playing...
                if (!GetComponent<AudioSource>().isPlaying)
                    // ... play them.
                    GetComponent<AudioSource>().Play();
            }
            else
                // Otherwise stop the footsteps.
                GetComponent<AudioSource>().Stop();

            // If the shout input has been pressed...
            if (shout)
            {
                // ... play the shouting clip where we are.
                AudioSource.PlayClipAtPoint(shoutingClip, transform.position);

                if (Vector3.Distance(enemy.position, transform.position) < 100.0f)
                {
                    enemySight.playerInSight = true;
                    lastPlayerSight.position = transform.position;
                }

                mNetworkMgr.SendPlayerShoutPacket(shout, transform.position);
            }
        }

    }
}