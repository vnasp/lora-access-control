import type { Metadata } from "next";
import { Outfit } from "next/font/google";
import "./globals.css";

const outfit = Outfit({
  variable: "--font-outfit",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "LoRa Access Control",
  description: "Sistema de control de acceso vehicular",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="es" className={`${outfit.variable} antialiased`}>
      <body className="flex h-screen flex-col font-(family-name:--font-outfit)">
        {children}
      </body>
    </html>
  );
}
