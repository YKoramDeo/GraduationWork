using UnityEngine;
using System.Collections;

public class EnemyAI : MonoBehaviour
{
    public float patrolSpeed = 5f;                          // The nav mesh agent's speed when patrolling.
    public float chaseSpeed = 20f;                           // The nav mesh agent's speed when chasing.
    public float chaseWaitTime = 5f;                        // The amount of time to wait when the last sighting is reached.
    public float patrolWaitTime = 1f;                       // The amount of time to wait when the patrol way point is reached.
    public Transform[] patrolWayPoints;                     // An array of transforms for the patrol route.
    public bool patrollBool = false;

    private EnemySight enemySight;                          // Reference to the EnemySight script.
    private NavMeshAgent nav;                               // Reference to the nav mesh agent.
    private Transform player;                               // Reference to the player's transform.
    private PlayerHealth playerHealth;                      // Reference to the PlayerHealth script.
    private LastPlayerSighting lastPlayerSighting;          // Reference to the last global sighting of the player.
    private EnemyAnimation enemyAnim;
    private float chaseTimer;                               // A timer for the chaseWaitTime.
    private float patrolTimer;                              // A timer for the patrolWaitTime.
    public int wayPointIndex;                              // A counter for the way point array.

    private NetworkMgr mNetworkMgr;
    private int mFrameCount;
    public Vector3 patrollpos;

    public bool connectNetwork;
    void Awake()
    {
        // Setting up the references.
        enemySight = GetComponent<EnemySight>();
        nav = GetComponent<NavMeshAgent>();
        player = GameObject.FindGameObjectWithTag(Tags.player).transform;
        playerHealth = player.GetComponent<PlayerHealth>();

        enemyAnim = GetComponent<EnemyAnimation>();

        patrollpos = Vector3.zero;
        connectNetwork = false;
    }

    void Start()
    {
        lastPlayerSighting = GameObject.FindGameObjectWithTag(Tags.gameController).GetComponent<LastPlayerSighting>();
        mNetworkMgr = NetworkMgr.GetInstance();
        mNetworkMgr.RegisterReceiveNotification(PacketType.MonsterSetInfo, OnReceiveMonsterSetInfo);
        //Get 몬스터의 현재 위치 서버로 부터 받는다.
        //transform.position = mNetworkMgr.GetMonsterPosition();
        //nav.Warp(mNetworkMgr.GetMonsterPosition());
    }

    void Update()
    {
        // If the player is in sight and is alive...
        //if (enemySight.playerInSight && playerHealth.health > 0f && Vector3.Distance(enemySight.personalLastSighting, transform.position)<2)
        if (playerHealth.health == 0f || enemyAnim.frontHitBool == true || enemyAnim.backHitBool == true)
            // ... bite.
            Bitting();

        // If the player has been sighted and isn't dead...
        else if (enemySight.personalLastSighting != lastPlayerSighting.resetPosition && playerHealth.health > 0f)
            // ... chase.
            Chasing();

        // Otherwise...
        else
            // ... patrol.
            Patrolling();
    }

    void FixedUpdate()
    {
        mFrameCount++;

        if (0 == mFrameCount % 10 && connectNetwork)
        {
            //mNetworkMgr.SendMonsterMovePacket(transform.position);  //몬스터 현재 위치 보냄
            mFrameCount = 0;
        }
    }

    private void OnReceiveMonsterSetInfo(PacketType type, byte[] packetData)
    {
        MonsterSetInfoPacket packet = new MonsterSetInfoPacket(packetData);
        MonsterSetInfoData data = packet.GetPacketData();

        Vector3 packetCurPos, packetPatrollPos;
        packetCurPos.x = data.posX;
        packetCurPos.y = data.posY;
        packetCurPos.z = data.posZ;
        packetPatrollPos.x = data.patrolPosX;
        packetPatrollPos.y = data.patrolPosY;
        packetPatrollPos.z = data.patrolPosZ;

        if (packetPatrollPos != patrollpos)
            patrollpos = packetPatrollPos;

        nav.Warp(packetCurPos);
        connectNetwork = true;

        Debug.Log("enter!!!! MonsterReceive!!!");
        mNetworkMgr.CompeleteConnect();
        return;
    }

    void Bitting()
    {
        patrollBool = false;
        // Stop the enemy where it is.
        nav.Stop();
        // Debug.Log("Now Bitting");
    }


    void Chasing()
    {
        patrollBool = false;
        //Debug.Log("Now Chasing");
        if (enemySight.playerInSight)
        {
            // Create a vector from the enemy to the last sighting of the player.
            Vector3 sightingDeltaPos = enemySight.personalLastSighting - transform.position;

            // If the the last personal sighting of the player is not close...
            if (sightingDeltaPos.sqrMagnitude > 4f)
                // ... set the destination for the NavMeshAgent to the last personal sighting of the player.
                // nav.destination = enemySight.personalLastSighting;
                nav.SetDestination(enemySight.personalLastSighting);

            // Set the appropriate speed for the NavMeshAgent.
            nav.speed = chaseSpeed;

            // If near the last personal sighting...
            if (nav.remainingDistance < nav.stoppingDistance)
            {
                // ... increment the timer.
                chaseTimer += Time.deltaTime;

                // If the timer exceeds the wait time...
                if (chaseTimer >= chaseWaitTime)
                {
                    // ... reset last global sighting, the last personal sighting and the timer.
                    lastPlayerSighting.position = lastPlayerSighting.resetPosition;
                    enemySight.personalLastSighting = lastPlayerSighting.resetPosition;
                    chaseTimer = 0f;
                }
            }
            else
                // If not near the last sighting personal sighting of the player, reset the timer.
                chaseTimer = 0f;
        }
    }


    void Patrolling()
    {
        patrollBool = true;
        //Debug.Log("Now patrolling");
        if (!enemySight.playerInSight)
        {
            // Set an appropriate speed for the NavMeshAgent.
            nav.speed = patrolSpeed;
            /*
            // If near the next waypoint or there is no destination...
            if (nav.destination == lastPlayerSighting.resetPosition || nav.remainingDistance < nav.stoppingDistance)
            {
                // ... increment the timer.
                patrolTimer += Time.deltaTime;

                // If the timer exceeds the wait time...
                if (patrolTimer >= patrolWaitTime)
                {
                    // ... increment the wayPointIndex.
                    if (wayPointIndex == patrolWayPoints.Length - 1)

                        wayPointIndex = 0;
                    else
                        wayPointIndex++;

                    // Reset the timer.
                    patrolTimer = 0;
                }
            }
            else
                // If not near a destination, reset the timer.
                patrolTimer = 0;
                */

            /*
        // Set the destination to the patrolWayPoint.
        nav.destination = patrolWayPoints[wayPointIndex].position;
        */

            nav.destination = patrollpos;
            // nav.SetDestination(nav.destination);
            // Debug.Log("Set Destination");
        }
    }
}