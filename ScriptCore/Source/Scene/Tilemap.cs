using System;

namespace HimiiEngine
{
    public class Tilemap : Component
    {
        public uint Width
        {
            get
            {
                InternalCalls.Tilemap_GetSize(Entity.ID, out uint width, out uint height);
                return width;
            }
            set
            {
                InternalCalls.Tilemap_GetSize(Entity.ID, out uint width, out uint height);
                InternalCalls.Tilemap_SetSize(Entity.ID, value, height);
            }
        }

        public uint Height
        {
            get
            {
                InternalCalls.Tilemap_GetSize(Entity.ID, out uint width, out uint height);
                return height;
            }
            set
            {
                InternalCalls.Tilemap_GetSize(Entity.ID, out uint width, out uint height);
                InternalCalls.Tilemap_SetSize(Entity.ID, width, value);
            }
        }

        public bool HasBounds => Width != 0 && Height != 0;

        public void GetBounds(out int minTileX, out int minTileY, out int maxTileX, out int maxTileY)
        {
            InternalCalls.Tilemap_GetBounds(Entity.ID, out minTileX, out minTileY, out maxTileX, out maxTileY);
        }

        public ushort GetTile(int tileX, int tileY)
        {
            return InternalCalls.Tilemap_GetTile(Entity.ID, tileX, tileY);
        }

        public void SetTile(int tileX, int tileY, ushort tileIdentifier)
        {
            InternalCalls.Tilemap_SetTile(Entity.ID, tileX, tileY, tileIdentifier);
        }
    }
}
