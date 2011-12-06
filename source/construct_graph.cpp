#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

const double MAX_DISTANCE = 20;

int dround(double);
double distance(double x1, double y1, double x2, double y2);
int minimal(double, double, double, double);
double minimum(double, double, double, double);
int sign(double);
double area(double P1x, double P1y, double P2x, double P2y);
double triangle_area(double Ax, double Ay, double Bx, double By, double Cx, double Cy);
bool quad_inside(double p1x,double p1y,double p2x,double p2y,double p3x,double p3y,double p4x,double p4y,double px,double py);

double curvature(double P0x, double P0y, double P1x, double P1y, double P2x, double P2y, double P3x, double P3y);
double* triangle_incenter(double Ax,double Ay,double Bx,double By,double Cx,double Cy);
double* find_intersection(double P1x, double P1y, double P2x, double P2y, double P3x, double P3y, double P4x, double P4y);
double* find_corner(double P1x, double P1y, double P2x, double P2y, double Ix, double Iy, vector<double*> Ctable);

double gradient(double P1x, double P1y, double P2x, double P2y, vector<double*> G);

//int main(int argc, char **argv) {
vector<double*>* construct_graph(vector<double*> Ltable, vector<double*> Ctable, vector<double*> grad_img, double WIDTH, double LAMBDA1, double LAMBDA2, bool use_corners) {

   double *dataread1, *dataread2;
   double *dataout;
   vector<double*> *Gtable = new vector<double*>;
   double MAX_W1 = 0, MAX_W2 = 0;
   ofstream output;

   if (use_corners) output.open("cornerdata");
   

   // PART 1 - SOLID EDGE CONSTRUCTION //

   for (int i=0; i<(int)Ltable.size(); i++) {

      dataout = new double[5];
      dataout[0] = 4*i;
      dataout[1] = 4*i + 2;
      dataout[2] = 0; 
      dataout[3] = 0;
      dataout[4] = 1; // solid
      (*Gtable).push_back(dataout);

      dataout = new double[5];
      dataout[0] = 4*i + 1;
      dataout[1] = 4*i + 3;
      dataout[2] = 0;
      dataout[3] = 0;
      dataout[4] = 1; // solid
      (*Gtable).push_back(dataout);
   }

   //////////////////////////////////////

   // PART 2 - DASHED EDGE CONSTRUCTION //

   double P1[2], back1[2], P2[2], back2[2];
   double M1[2], M2[2], *C, *I, curv;
   double D, w1, area1, area2, carea, gaparea;
   double G1, G2, gapG;
   int k, n1, n2, count_dashed = 0;

   for (int i=0; i<(int)Ltable.size()-1; i++) {
      dataread1 = Ltable[i];

      for (int j=i+1; j<(int)Ltable.size(); j++) {
         dataread2 = Ltable[j];

         // find closest axis endpoints.
         D = minimum(distance(dataread1[0],dataread1[1],dataread2[0],dataread2[1]),distance(dataread1[0],dataread1[1],dataread2[2],dataread2[3]),distance(dataread1[2],dataread1[3],dataread2[0],dataread2[1]),distance(dataread1[2],dataread1[3],dataread2[2],dataread2[3]));
         k = minimal(distance(dataread1[0],dataread1[1],dataread2[0],dataread2[1]),distance(dataread1[0],dataread1[1],dataread2[2],dataread2[3]),distance(dataread1[2],dataread1[3],dataread2[0],dataread2[1]),distance(dataread1[2],dataread1[3],dataread2[2],dataread2[3]));
         // D is min distance, and k is index 1-4, e.g. 1 means O_2i and O_2j

         if (D > MAX_DISTANCE) continue;


         if (k < 3) {
            P1[0] = dataread1[0];
            P1[1] = dataread1[1];
            back1[0] = dataread1[2];
            back1[1] = dataread1[3];
         } else {
            P1[0] = dataread1[2];
            P1[1] = dataread1[3];
            back1[0] = dataread1[0];
            back1[1] = dataread1[1];
         }
         n1 = 2*i;
         area1 = area(P1[0],P1[1],back1[0],back1[1]);
         //G1 = gradient(P1[0],P1[1],back1[0],back1[1], grad_img);
         if (P1[0] > back1[0])
            n1++;

         if (k % 2 == 1) {
            P2[0] = dataread2[0];
            P2[1] = dataread2[1];
            back2[0] = dataread2[2];
            back2[1] = dataread2[3];
         } else {
            P2[0] = dataread2[2];
            P2[1] = dataread2[3];
            back2[0] = dataread2[0];
            back2[1] = dataread2[1];
         }
         n2 = 2*j;
         area2 = area(P2[0],P2[1],back2[0],back2[1]);
         //G2 = gradient(P2[0],P2[1],back2[0],back2[1],grad_img);
         if (P2[0] > back2[0])
            n2++;

         if (use_corners) {
            I = find_intersection(P1[0],P1[1],back1[0],back1[1],P2[0],P2[1],back2[0],back2[1]);
            C = find_corner(P1[0],P1[1],P2[0],P2[1],I[0],I[1], Ctable);
         } else {
            C = new double[2];
            C[0] = -1; C[1] = -1;
         }
         
         // calculate W1
         if (C[0] < 0) {
            M1[0] = (P1[0]+back1[0])/2;
            M2[0] = (P2[0]+back2[0])/2;
            if (LAMBDA1 > 0) {
            curv = curvature(M1[0],M1[1],P1[0],P1[1],P2[0],P2[1],M2[0],M2[1]);
            if (curv > 40) curv = 40; // avoid spikes in curvature
            } else curv = 0;
            w1 = distance(P1[0],P1[1],P2[0],P2[1]) + LAMBDA1*curv;
         } else {
            w1 = distance(P1[0],P1[1],C[0],C[1]) + distance(C[0],C[1],P2[0],P2[1]);
         }

         // calculate W2
         gaparea = area(P1[0],P1[1],P2[0],P2[1]);
         if (C[0] < 0)
            carea = 0;
         else
            carea = triangle_area(P1[0],P1[1],C[0],C[1],P2[0],P2[1]);
         //gapG = gradient(P1[0],P1[1],P2[0],P2[1], grad_img);
         if (P1[0] > P2[0]) {
            gaparea = -gaparea;
            carea = -carea;
            //gapG = -gapG;
         }
         if (use_corners) delete I;

         // create dashed edge
         if (P1[0] > back1[0]) {

            dataout = new double[5];
            dataout[0] = 2*n1;

            if (P2[0] > back2[0]) {
               dataout[1] = 2*n2 + 1;
               dataout[2] = (area1 - area2)/2 + gaparea + carea; // + LAMBDA2*((G1 - G2)/2 + gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);

               dataout = new double[5];
               dataout[0] = 2*n1 + 1;
               dataout[1] = 2*n2;
               dataout[2] = (-area1 + area2)/2 - gaparea - carea; // + LAMBDA2*((-G1 + G2)/2 - gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);
               if (C[0] > 0) {
                  output << 2*n1 << " " << 2*n2+1 << " " << C[0] << " " << C[1] << endl;
                  output << 2*n1+1 << " " << 2*n2 << " " << C[0] << " " << C[1] << endl;
               }

            }
            else {
               dataout[1] = 2*n2;
               dataout[2] = (area1 + area2)/2 + gaparea + carea; // + LAMBDA2*((G1 + G2)/2 + gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);

               dataout = new double[5];
               dataout[0] = 2*n1 + 1;
               dataout[1] = 2*n2 + 1;
               dataout[2] = (-area1 - area2)/2 - gaparea - carea; // + LAMBDA2*((-G1 - G2)/2 - gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);
               if (C[0] > 0) {
                  output << 2*n1 << " " << 2*n2 << " " << C[0] << " " << C[1] << endl;
                  output << 2*n1+1 << " " << 2*n2+1 << " " << C[0] << " " << C[1] << endl;
               }

            }

         } else {

            dataout = new double[5];
            dataout[0] = 2*n1 + 1;

            if (P2[0] > back2[0]) {
               dataout[1] = 2*n2 + 1;
               dataout[2] = (-area1 - area2)/2 + gaparea + carea; // + LAMBDA2*((-G1 - G2)/2 + gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);

               dataout = new double[5];
               dataout[0] = 2*n1;
               dataout[1] = 2*n2;
               dataout[2] = (area1 + area2)/2 - gaparea - carea; // + LAMBDA2*((G1 + G2)/2 - gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);
               if (C[0] > 0) {
                  output << 2*n1+1 << " " << 2*n2+1 << " " << C[0] << " " << C[1] << endl;
                  output << 2*n1 << " " << 2*n2 << " " << C[0] << " " << C[1] << endl;
               }

            }
            else {
               dataout[1] = 2*n2;
               dataout[2] = (-area1 + area2)/2 + gaparea + carea; // + LAMBDA2*((-G1 + G2)/2 + gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);

               dataout = new double[5];
               dataout[0] = 2*n1;
               dataout[1] = 2*n2 + 1;
               dataout[2] = (area1 - area2)/2 - gaparea - carea; // + LAMBDA2*((G1 - G2)/2 - gapG);
               dataout[3] = w1;
               dataout[4] = 0; // dashed
               (*Gtable).push_back(dataout);
               if (C[0] > 0) {
                  output << 2*n1+1 << " " << 2*n2 << " " << C[0] << " " << C[1] << endl;
                  output << 2*n1 << " " << 2*n2+1 << " " << C[0] << " " << C[1] << endl;
               }

            }

         }

         delete C;

         count_dashed += 2;
         if (w1 > MAX_W1) MAX_W1 = w1;
         if (fabs(dataout[2]) > MAX_W2) MAX_W2 = fabs(dataout[2]);
      }
   }

   if (use_corners) output.close();

   cout << "Dashed edges: " << count_dashed << endl;

   dataout = new double[5];
   dataout[0] = MAX_W2;
   dataout[1] = MAX_W1;
   dataout[2] = 0;
   dataout[3] = 0;
   dataout[4] = 1;
   (*Gtable).push_back(dataout);

   cout << "MAX_W1: " << MAX_W1 << ", MAX_W2:" << MAX_W2 << endl;


   return Gtable;
}


double distance(double x1, double y1, double x2, double y2) {
   return sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}

int minimal(double d1, double d2, double d3, double d4) {
   if (d1 <= d2 && d1 <= d3 && d1 <= d4){return 1;}
   if (d2 <= d1 && d2 <= d3 && d2 <= d4){return 2;}
   if (d3 <= d1 && d3 <= d2 && d3 <= d4){return 3;}
   if (d4 <= d1 && d4 <= d2 && d4 <= d3){return 4;}
   return 0;
}

double minimum(double d1, double d2, double d3, double d4) {
   if (d1 <= d2 && d1 <= d3 && d1 <= d4){return d1;}
   if (d2 <= d1 && d2 <= d3 && d2 <= d4){return d2;}
   if (d3 <= d1 && d3 <= d2 && d3 <= d4){return d3;}
   if (d4 <= d1 && d4 <= d2 && d4 <= d3){return d4;}
   return 0;
}

int sign(double d) {
   if (d == 0) return 0;
   return ((d>0)?1:-1);
}


int dround(double d) {
   return ((int)floor(d + 0.5));
}



double area(double P1x, double P1y, double P2x, double P2y) {

   return ( fabs(P1x-P2x)*( (P1y>P2y ? P1y : P2y) - fabs(P1y-P2y)/2) );

}

double triangle_area(double Ax, double Ay, double Bx, double By, double Cx, double Cy) {

   return ( fabs(-Bx*Ay + Cx*Ay + Ax*By - Cx*By - Ax*Cy + Bx*Cy)/2 );

}



bool quad_inside(double p1x,double p1y,double p2x,double p2y,double p3x,double p3y,double p4x,double p4y,double px,double py) {

   int s1, s2, s3;

   // first, check if p inside triangle p1p2p3

   s1 = sign(-p2x*p1y + px*p1y + p1x*p2y - px*p2y - p1x*py + p2x*py);
   s2 = sign(-p3x*p2y + px*p2y + p2x*p3y - px*p3y - p2x*py + p3x*py);
   s3 = sign(-p1x*p3y + px*p3y + p3x*p1y - px*p1y - p3x*py + p1x*py);

   if ((s1 == s2) && (s1 == s3)) { // point is inside
      return true;
   } else { // check second triangle, p3p4p1

      s1 = sign(-p3x*p1y + px*p1y + p1x*p3y - px*p3y - p1x*py + p3x*py);
      s2 = sign(-p4x*p3y + px*p3y + p3x*p4y - px*p4y - p3x*py + p4x*py);
      s3 = sign(-p1x*p4y + px*p4y + p4x*p1y - px*p1y - p4x*py + p1x*py);

      if ((s1 == s2) && (s1 == s3))
         return true;
      else
         return false;

   }

}

double gradient(double P1x, double P1y, double P2x, double P2y, vector<double*> G) {

   double sum = 0, *line, m, b;
   int minx, maxx, maxy;

   if (P1x < P2x) {
      minx = dround(P1x);
      maxx = dround(P2x);
   } else {
      minx = dround(P2x);
      maxx = dround(P1x);
   }

   if (minx == maxx) return sum;

   m = (P1y-P2y)/(P1x-P2x);
   b = P1y - m*P1x;

   for (int i=minx; i<=maxx; i++) {

      maxy = dround(m*i + b);

      for (int j=0; j <= maxy; j++) {
         line = G[j];
         sum += line[i];
      }

   }

   return sum;

}

double* find_intersection(double P1x, double P1y, double P2x, double P2y, double P3x, double P3y, double P4x, double P4y) {

   double *I = new double[2];
   double m1,m2,b1,b2;

   if (P1x == P2x)
      m1 = 0;
   else {
      m1 = (P1y - P2y)/(P1x - P2x);
      b1 = P1y - m1*P1x;
   }
   if (P3x == P4x)
      m2 = 0;
   else {
      m2 = (P3y - P4y)/(P3x - P4x);
      b2 = P3y - m2*P3x;
   }

   if (P1x == P2x) {
      if (P3x == P4x) { //parallel, no intersection
         I[0] = -1;
         I[1] = -1;
      } else {
         I[0] = P1x;
         I[1] = m2*I[0] + b2;
      }
   } else {
      if (P3x == P4x) {
         I[0] = P3x;
         I[1] = m1*I[0] + b1;
      } else {
         if (m1 == m2) { //parallel, no intersection
            I[0] = -1;
            I[1] = -1;
         } else {
            I[0] = (b2-b1)/(m1-m2);
            I[1] = m1*I[0] + b1;
         }
      }
   }

   // discard I if P1,P2 and I are aligned
   if (((I[0] - P1x) < 0.01) && ((I[1] - P1y) < 0.01)) {
      I[0] = -1; I[1] = -1;
   } else if (((I[0] - P3x) < 0.01) && ((I[1] - P3y) < 0.01)) {
      I[0] = -1; I[1] = -1;
   }

   return I;
}

double* find_corner(double P1x, double P1y, double P2x, double P2y, double Ix, double Iy, vector<double*> Ctable) {

   double *C = new double[2], *p;
   double c1[2],c2[2],c3[2],c4[2];
   double m,b1,b2, d = 0;

   C[0] = -1;
   C[1] = -1;

   if (Ix == -1) {
      return C;
   }

   if (distance(P1x,P1y,Ix,Iy) > distance(P2x,P2y,Ix,Iy)) {
      c1[0] = P1x;
      c1[1] = P1y;
   } else {
      c1[0] = P2x;
      c1[1] = P2y;
   }

   double *inctr = triangle_incenter(P1x,P1y,Ix,Iy,P2x,P2y);

   if (Ix == inctr[0]) { //angle bisector is vertical
      c2[0] = 2*Ix - c1[0];
      c2[1] = c1[1];
   } else {
      m = (Iy-inctr[1])/(Ix-inctr[0]);
      if (m == 0) {
         c2[0] = c1[0];
         c2[1] = 2*Iy - c1[1];
      } else {
         b1 = c1[1] + c1[0]/m;
         b2 = Iy - m*Ix;
         c2[0] = (b2-b1)/(-1/m - m);
         c2[1] = m*c2[0] + b2;
      }
   }

   c3[0] = 2*Ix - c1[0];
   c3[1] = 2*Iy - c1[1];
   c4[0] = 2*Ix - c2[0];
   c4[1] = 2*Iy - c2[1];

/*if (P1x == 84 && P2x == 84) {
cout << "quad" << endl;
cout << c1[0] << " " << c1[1] << endl;
cout << c2[0] << " " << c2[1] << endl;
cout << c3[0] << " " << c3[1] << endl;
cout << c4[0] << " " << c4[1] << endl;
}*/

   //Search area is {c1,c2,c3,c4} 

   for (int i=0; i<(int)Ctable.size(); i++) {

      p = Ctable[i];

      if (p[2] > d) {
         if (quad_inside(c1[0],c1[1],c2[0],c2[1],c3[0],c3[1],c4[0],c4[1],p[0],p[1])) {
            C[0] = p[0];
            C[1] = p[1];
            d = p[2];
         }
      }
   }

   return C;

}


double* triangle_incenter(double Ax,double Ay,double Bx,double By,double Cx,double Cy) {

   double a,b,c, perimeter;
   double *center = new double[2];

   a = distance(Ax,Ay,Bx,By);
   b = distance(Bx,By,Cx,Cy);
   c = distance(Cx,Cy,Ax,Ay);

   perimeter = a+b+c;

   if ( perimeter == 0) {
      center[0] = Ax;
      center[1] = Ay;
   } else {
      center[0] = (b*Ax + c*Bx + a*Cx)/perimeter;
      center[1] = (b*Ay + c*By + a*Cy)/perimeter;
   }

   return center;
}


double curvature(double P0x, double P0y, double P1x, double P1y, double P2x, double P2y, double P3x, double P3y) {

    double total = 0, delta, x, xx, y, yy, K;
    double interm[4][2];
    int n = 100;
    double length, u, x1, y1, x2, y2;

    delta = 1.0 / n;

    interm[0][0] = -(P0x) + (3 * P1x) - (3 * P2x) + P3x;
    interm[0][1] = -(P0y) + (3 * P1y) - (3 * P2y) + P3y;
    interm[1][0] = (3 * P0x) - (6 * P1x) + (3 * P2x);
    interm[1][1] = (3 * P0y) - (6 * P1y) + (3 * P2y);
    interm[2][0] = -(3 * P0x) + (3 * P1x);
    interm[2][1] = -(3 * P0y) + (3 * P1y);
    interm[3][0] = P0x;
    interm[3][1] = P0y;

    for (int i = 0; i < n; i++)
    {
        x = 3 * (delta * delta * i * i) * interm[0][0] + 2 * delta * i * interm[1][0] + interm[2][0];
        xx = 6 * delta * i * interm[0][0] + 2 * interm[1][0];
        y = 3 * (delta * delta * i * i) * interm[0][1] + 2 * delta * i * interm[1][1] + interm[2][1];
        yy = 6 * delta * i * interm[0][1] + 2 * interm[1][1];

        if ( (x == 0) && (y == 0) )
            K = 0;
        else
        {
            K = ( fabs(x * yy - xx * y) / pow((fabs(x * x + y * y)), 1.5) );
            K = K * K;
        }

        u = delta*i;
        x1 = interm[0][0] * u * u * u + interm[1][0] * u * u + interm[2][0] * u + interm[3][0];
        y1 = interm[0][1] * u * u * u + interm[1][1] * u * u + interm[2][1] * u + interm[3][1];
        u = delta*(i+1);
        x2 = interm[0][0] * u * u * u + interm[1][0] * u * u + interm[2][0] * u + interm[3][0];
        y2 = interm[0][1] * u * u * u + interm[1][1] * u * u + interm[2][1] * u + interm[3][1];

        length = distance(x1,y1,x2,y2);

        total += K * length;
    }

    return total;

}

