using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public struct Vector3
    {
        static Random random;

        static Vector3()
        {
            random = new Random();
        }

        public Vector3(float x, float y, float z)
            : this()
        {
            this.X = x;
            this.Y = y;
            this.Z = z;
        }

        public override string ToString()
        {
            return string.Format("({0}, {1}, {2})", X, Y, Z);
        }

        public float DistanceTo2D(Vector3 other)
        {
            Vector3 temp = other;
            temp.Z = 0;

            Vector3 temp2 = this;
            temp2.Z = 0;

            return temp2.DistanceTo(temp);
        }

        public float DistanceTo(Vector3 other)
        {
            Vector3 diff = other - this;

            return diff.Length();
        }

        public float Length()
        {
            return (float)Math.Sqrt((X * X) + (Y * Y) + (Z * Z));
        }

        public Vector3 Around(float distance)
        {
            Vector3 difference = Vector3.RandomXY() * distance;

            return (this + difference);
        }

        public static Vector3 RandomXY()
        {
            Vector3 retval = new Vector3((float)(random.NextDouble() - 0.5), (float)(random.NextDouble() - 0.5), 0f);
            retval.Normalize();

            return retval;
        }

        public void Normalize()
        {
            var length = Length();

            if (length > 0)
            {
                var factor = 1.0f / length;
                X = X * factor;
                Y = Y * factor;
                Z = Z * factor;
            }
        }

        public static Vector3 operator -(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
        }

        public static Vector3 operator +(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
        }

        public static Vector3 operator *(Vector3 vector, float factor)
        {
            return new Vector3(vector.X * factor, vector.Y * factor, vector.Z * factor);
        }

        public float X;
        public float Y;
        public float Z;
    }
}
