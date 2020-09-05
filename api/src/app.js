require('dotenv').config()

const os = require('os')
const dns = require('dns')
const axios = require('axios')
const express = require('express')

const fs = require('fs')

let ipAddress = ''
dns.lookup(os.hostname(), (err, add, fam) => {
  ipAddress = add
})
const port = 3000

const app = express()

const passport = require('passport')
const KakaoStrategy = require('passport-kakao').Strategy

const multer = require('multer')
const path = require('path')
const Jimp = require('jimp')
const { send } = require('process')

const appKey = process.env.API_KEY
const appSecret = process.env.CLIENT_SECRET_KEY

const dashboardUrl = process.env.DASHBOARD_URL

axios.defaults.baseURL = 'https://kapi.kakao.com/v2/api'
axios.defaults.headers.post['Content-Type'] = 'application/x-www-form-urlencoded'

const user = {}
const imagePath = './public/images'
const storage = multer.diskStorage({
  destination: function(req, file, cb) {
    cb(null, imagePath);
  },
  filename: (req, file, cb) => {
    console.log(file); //log the file object info in console

    user.photo = {}
    const image = new Jimp([imagePath, file.originalname].join('/'), (err, image) => {
      user.photo.width = image.bitmap.width
      user.photo.height = image.bitmap.height
  });
    user.photo.filename = file.originalname
    cb(null, file.originalname);//here we specify the file saving name. in this case. 
//i specified the original file name .you can modify this name to anything you want
  }
});

const uploadDisk = multer({ storage: storage });

passport.use(
  new KakaoStrategy(
    {
      clientID: appKey,
      clientSecret: appSecret,
      callbackURL: `http://localhost:${port}/oauth`,
    },
    function (accessToken, refreshToken, params, profile, done) {
      console.log(`accessToken : ${accessToken}`)
      console.log(`사용자 profile: ${JSON.stringify(profile._json)}`)

      save(accessToken, refreshToken, profile)
      return done(null, profile._json)
    }
  )
)

passport.serializeUser(function (user, done) {
  done(null, user)
})
passport.deserializeUser(function (obj, done) {
  done(null, obj)
})

app.use(passport.initialize())
app.use('/static', express.static('public'))

app.get('/', (req, res) => {
  res.send('Hello World!')
})

app.get('/login', passport.authenticate('kakao', { state: 'myStateValue' }))
app.get('/oauth', passport.authenticate('kakao'), function (req, res) {
  console.log(req.user)
  res.send('state :' + req.query.state)
})

app.get('/sendOnly', (req, res) => {
  res.send(sendPushMessage() ? 'ok' : 'nok')
})

app.get('/send', (req, res) => {
  res.send(allInOne() ? 'ok' : 'nok')
})

app.post('/upload', uploadDisk.single('photo'), (req, res) => {
  res.send('file disk upload success.');
})

app.post('/pingpong', (req, res) => {
  res.send('ping pong')
})

const on = async() => {
  await axios.get(`${dashboardUrl}/command/on`)
}
const off = async() => {
  await axios.get(`${dashboardUrl}/command/off`)
}

app.get('/on', async(req, res) => {
  await on()
  res.send('ok')
})

app.get('/off', async(req, res) => {
  await off()
  res.send('ok')
})

const latest = async() => {
  const {data: base64Image} = await axios.get(`${dashboardUrl}/latest`)
  await fs.writeFileSync('public/images/latest.jpg', base64Image, {encoding: 'base64'}, function(err) {
    console.log('File created')
  })
}

app.get('/latest', async(req, res) => {
  await latest()
  res.setHeader('Content-Type', 'image/jpeg')
  res.send(fs.readFileSync('public/images/latest.jpg'))
  // res.redirect('static/images/latest.jpg')
})

const sendPushMessage = async() => {
  if (!user.accessToken) {
    return false
  }
  console.log(user.accessToken)
  axios.defaults.headers.common['Authorization'] = `Bearer ${user.accessToken}`
  user.photo = {}

  // "image_url": "${ipAddress}:${port}/static/images/${user.photo.filename}",
  templateObject = `{
    "object_type": "feed",
    "content": {
      "title": "택배가 도착했어요!",
      "description": "${ipAddress}:${port}/static/images/latest.jpg",
      "image_url": "https://pixy.org/src/428/4288090.png",
      "image_width": ${user.photo.width ? user.photo.width : 640},
      "image_height": ${user.photo.height ? user.photo.height : 640},
      "link": {
        "web_url": "${dashboardUrl}",
        "mobile_web_url": "${dashboardUrl}"
      }
    }
  }`

  console.log(templateObject)
  
  axios.post('/talk/memo/default/send', 'template_object=' + templateObject)
    .catch(function (error) {
      console.log(error)
      return false
    })
  return true
}

const allInOne = async() => {
  await sleep(1000)
  await on()
  await sleep(1000)
  await latest()
  await sendPushMessage()
  await sleep(1000)
  await off()
  return true
}

const save = (accessToken, refreshToken, profile) => {
  user.accessToken = accessToken
  user.refreshToken = refreshToken
  user.profile = profile
}

function sleep(ms) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms)
  })
} 

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})