using UnityEngine;
using System.Collections;

public class TeddyBearPickUp : MonoBehaviour
{
    public AudioClip keyGrab;                       // Audioclip to play when the key is picked up.
    public GameObject bearPanel;
    public UILabel alarmText;

    private GameObject player;                      // Reference to the player.
    private PlayerInventory playerInventory;        // Reference to the player's inventory.
    private SceneFadeInOut sceneFadeInOut;

    public int itemID;
    private NetworkMgr mNetwork;

    void Awake()
    {
        // Setting up the references.
        player = GameObject.FindGameObjectWithTag(Tags.player);
        playerInventory = player.GetComponent<PlayerInventory>();
        sceneFadeInOut = GameObject.FindGameObjectWithTag(Tags.fader).GetComponent<SceneFadeInOut>();
    }


    void Start()
    {
        mNetwork = NetworkMgr.GetInstance();
        itemID = 7;
    }

    void OnTriggerEnter(Collider other)
    {
        // If the colliding gameobject is the player...
        if (other.gameObject == player)
        {
            if (player.GetComponent<PlayerHealth>().tutorialState == false)
            {
                bearPanel.SetActive(true);
                // ... play the clip at the position of the key...
                AudioSource.PlayClipAtPoint(keyGrab, transform.position);

                // ... the player has a key ...
                ++playerInventory.teddyBear_count;

                mNetwork.SendPlayerGetItemPacket(itemID);

                //모든 곰인형을 모았을 때 hasKey 를 true 시켜서 게임을 로고로 보낸다.
                if (playerInventory.teddyBear_count == 6)
                {
                    playerInventory.hasKey = true;
                    sceneFadeInOut.EndScene();
                }

                SoundManager.instance.Play(106);
                SoundManager.instance.Play(0);
                alarmText.text = "곰인형의 일부분을 획득 하였습니다.";
                alarmText.enabled = true;
                GetComponent<ItemGetSprite>().TurnOn();
                ItemData.instance.Bear();

                // ... and destroy this gameobject.
                this.gameObject.SetActive(false);
            }
            else
            {
                bearPanel.SetActive(true);
                // ... play the clip at the position of the key...
                AudioSource.PlayClipAtPoint(keyGrab, transform.position);

                SoundManager.instance.Play(106);
                SoundManager.instance.Play(0);
                alarmText.text = "곰인형의 일부분을 획득 하였습니다.";
                alarmText.enabled = true;
                GetComponent<ItemGetSprite>().TurnOn();
                ItemData.instance.Bear();

                // ... and destroy this gameobject.
                //this.gameObject.SetActive(false);
                Destroy(this.gameObject);
            }
        }
    }
}