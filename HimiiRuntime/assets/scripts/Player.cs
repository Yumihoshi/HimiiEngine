using HimiiEngine;

public class Player:Entity
{
    public override void OnCreate()
    {
        base.OnCreate();
        Console.WriteLine("Player created");
    }

    public override void OnUpdate(float deltaTime)
    {
        base.OnUpdate(deltaTime);
        
        if(Input.IsKeyDown(KeyCode.Space))
        {
            if (HasComponent<Rigidbody2D>())
            {
                var rigidbody = GetComponent<Rigidbody2D>();
                rigidbody.ApplyImpulse(new Vector2(0, 1f), true);
            }
        }
    }
}